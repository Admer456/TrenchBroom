/*
 Copyright (C) 2010 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Error.h"
#include "Exceptions.h"
#include "FloatType.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Game.h"
#include "Model/Hit.h"
#include "Model/Polyhedron.h"
#include "Model/Polyhedron3.h"
#include "Model/WorldNode.h"
#include "NotifierConnection.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderService.h"
#include "View/BrushVertexCommands.h"
#include "View/Lasso.h"
#include "View/MapDocument.h"
#include "View/Selection.h"
#include "View/Tool.h"
#include "View/TransactionScope.h"
#include "View/VertexHandleManager.h"

#include "kdl/memory_utils.h"
#include "kdl/overload.h"
#include "kdl/result.h"
#include "kdl/set_temp.h"
#include "kdl/string_utils.h"
#include "kdl/vector_set.h"
#include "kdl/vector_utils.h"

#include "vm/forward.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class PickResult;
}

namespace Renderer
{
class Camera;
}

namespace View
{
class Grid;
class Lasso;
class MapDocument;

template <typename H>
class VertexToolBase : public Tool
{
public:
  enum class MoveResult
  {
    Continue,
    Deny,
    Cancel
  };

protected:
  std::weak_ptr<MapDocument> m_document;

private:
  size_t m_changeCount;
  size_t m_ignoreChangeNotifications;
  NotifierConnection m_notifierConnection;

protected:
  H m_dragHandlePosition;
  bool m_dragging;

protected:
  explicit VertexToolBase(std::weak_ptr<MapDocument> document)
    : Tool(false)
    , m_document(std::move(document))
    , m_changeCount(0)
    , m_ignoreChangeNotifications(0u)
    , m_dragging(false)
  {
  }

public:
  ~VertexToolBase() override = default;

public:
  const Grid& grid() const { return kdl::mem_lock(m_document)->grid(); }

  const std::vector<Model::BrushNode*>& selectedBrushes() const
  {
    auto document = kdl::mem_lock(m_document);
    return document->selectedNodes().brushes();
  }

public:
  template <typename M, typename I>
  std::map<typename M::Handle, std::vector<Model::BrushNode*>> buildBrushMap(
    const M& manager, I cur, I end) const
  {
    using H2 = typename M::Handle;
    std::map<H2, std::vector<Model::BrushNode*>> result;
    while (cur != end)
    {
      const H2& handle = *cur++;
      result[handle] = findIncidentBrushes(manager, handle);
    }
    return result;
  }

  // FIXME: use vector_set
  template <typename M, typename H2>
  std::vector<Model::BrushNode*> findIncidentBrushes(
    const M& manager, const H2& handle) const
  {
    const std::vector<Model::BrushNode*>& brushes = selectedBrushes();
    return manager.findIncidentBrushes(handle, std::begin(brushes), std::end(brushes));
  }

  // FIXME: use vector_set
  template <typename M, typename I>
  std::vector<Model::BrushNode*> findIncidentBrushes(const M& manager, I cur, I end) const
  {
    const std::vector<Model::BrushNode*>& brushes = selectedBrushes();
    kdl::vector_set<Model::BrushNode*> result;
    auto out = std::inserter(result, std::end(result));

    while (cur != end)
    {
      const auto& handle = *cur;
      manager.findIncidentBrushes(handle, std::begin(brushes), std::end(brushes), out);
      ++cur;
    }

    return result.release_data();
  }

  virtual void pick(
    const vm::ray3& pickRay,
    const Renderer::Camera& camera,
    Model::PickResult& pickResult) const = 0;

public: // Handle selection
  bool select(const std::vector<Model::Hit>& hits, const bool addToSelection)
  {
    assert(!hits.empty());
    const Model::Hit& firstHit = hits.front();
    if (firstHit.type() == handleManager().hitType())
    {
      if (!addToSelection)
        handleManager().deselectAll();

      // Count the number of hit handles which are selected already.
      size_t selected = 0u;
      for (const auto& hit : hits)
      {
        if (handleManager().selected(hit.target<H>()))
        {
          ++selected;
        }
      }

      if (selected < hits.size())
      {
        for (const auto& hit : hits)
        {
          handleManager().select(hit.target<H>());
        }
      }
      else if (addToSelection)
      {
        // The user meant to deselect a selected handle.
        for (const auto& hit : hits)
        {
          handleManager().deselect(hit.target<H>());
        }
      }
    }
    refreshViews();
    notifyToolHandleSelectionChanged();
    return true;
  }

  void select(const Lasso& lasso, const bool modifySelection)
  {
    using HandleList = std::vector<H>;

    const HandleList allHandles = handleManager().allHandles();
    HandleList selectedHandles;

    lasso.selected(
      std::begin(allHandles), std::end(allHandles), std::back_inserter(selectedHandles));
    if (!modifySelection)
    {
      handleManager().deselectAll();
    }
    handleManager().toggle(std::begin(selectedHandles), std::end(selectedHandles));
    refreshViews();
    notifyToolHandleSelectionChanged();
  }

  bool selected(const Model::Hit& hit) const
  {
    return handleManager().selected(hit.target<H>());
  }

  virtual bool deselectAll()
  {
    if (handleManager().anySelected())
    {
      handleManager().deselectAll();
      refreshViews();
      notifyToolHandleSelectionChanged();
      return true;
    }
    return false;
  }

public:
  using HandleManager = VertexHandleManagerBaseT<H>;
  virtual HandleManager& handleManager() = 0;
  virtual const HandleManager& handleManager() const = 0;

public: // performing moves
  virtual std::tuple<vm::vec3, vm::vec3> handlePositionAndHitPoint(
    const std::vector<Model::Hit>& hits) const = 0;

  virtual bool startMove(const std::vector<Model::Hit>& hits)
  {
    assert(!hits.empty());

    // Delesect all handles if any of the hit handles is not already selected.
    for (const auto& hit : hits)
    {
      const H handle = getHandlePosition(hit);
      if (!handleManager().selected(handle))
      {
        handleManager().deselectAll();
        break;
      }
    }

    // Now select all of the hit handles.
    for (const auto& hit : hits)
    {
      const H handle = getHandlePosition(hit);
      if (hit.hasType(handleManager().hitType()))
      {
        handleManager().select(handle);
      }
    }
    refreshViews();

    auto document = kdl::mem_lock(m_document);
    document->startTransaction(actionName(), TransactionScope::LongRunning);

    m_dragHandlePosition = getHandlePosition(hits.front());
    m_dragging = true;
    ++m_ignoreChangeNotifications;
    return true;
  }

  virtual MoveResult move(const vm::vec3& delta) = 0;

  virtual void endMove()
  {
    auto document = kdl::mem_lock(m_document);
    document->commitTransaction();
    m_dragging = false;
    --m_ignoreChangeNotifications;
  }

  virtual void cancelMove()
  {
    auto document = kdl::mem_lock(m_document);
    document->cancelTransaction();
    m_dragging = false;
    --m_ignoreChangeNotifications;
  }

  virtual bool allowAbsoluteSnapping() const
  {
    // override in VertexTool
    return false;
  }

public: // csg convex merge
  bool canDoCsgConvexMerge() { return handleManager().selectedHandleCount() > 1; }

  void csgConvexMerge()
  {
    auto vertices = std::vector<vm::vec3>{};
    const auto handles = handleManager().selectedHandles();
    H::get_vertices(std::begin(handles), std::end(handles), std::back_inserter(vertices));

    const auto polyhedron = Model::Polyhedron3{vertices};
    if (!polyhedron.polyhedron() || !polyhedron.closed())
    {
      return;
    }

    auto document = kdl::mem_lock(m_document);
    auto game = document->game();

    const auto builder = Model::BrushBuilder{
      document->world()->mapFormat(),
      document->worldBounds(),
      game->config().faceAttribsConfig.defaults};
    builder.createBrush(polyhedron, document->currentMaterialName())
      | kdl::transform([&](auto b) {
          for (const auto* selectedBrushNode : document->selectedNodes().brushes())
          {
            b.cloneFaceAttributesFrom(selectedBrushNode->brush());
          }

          auto* newParent = document->parentForNodes(document->selectedNodes().nodes());
          auto transaction = Transaction{document, "CSG Convex Merge"};
          deselectAll();
          if (document->addNodes({{newParent, {new Model::BrushNode{std::move(b)}}}})
                .empty())
          {
            transaction.cancel();
            return;
          }
          transaction.commit();
        })
      | kdl::transform_error(
        [&](auto e) { document->error() << "Could not create brush: " << e.msg; });
  }

  virtual H getHandlePosition(const Model::Hit& hit) const
  {
    assert(hit.isMatch());
    assert(hit.hasType(handleManager().hitType()));
    return hit.target<H>();
  }

  virtual std::string actionName() const = 0;

public:
  void moveSelection(const vm::vec3& delta)
  {
    const auto ignoreChangeNotifications = kdl::inc_temp{m_ignoreChangeNotifications};

    auto transaction = Transaction{m_document, actionName()};
    move(delta);
    transaction.commit();
  }

  bool canRemoveSelection() const { return handleManager().selectedHandleCount() > 0; }

public: // rendering
  void renderHandles(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
  {
    Renderer::RenderService renderService(renderContext, renderBatch);
    if (!handleManager().allSelected())
    {
      renderHandles(
        handleManager().unselectedHandles(),
        renderService,
        pref(Preferences::HandleColor));
    }
    if (handleManager().anySelected())
    {
      renderHandles(
        handleManager().selectedHandles(),
        renderService,
        pref(Preferences::SelectedHandleColor));
    }
  }

  void renderDragHandle(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
  {
    renderHandle(
      renderContext,
      renderBatch,
      m_dragHandlePosition,
      pref(Preferences::SelectedHandleColor));
  }

  template <typename HH>
  void renderHandle(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const HH& handle) const
  {
    renderHandle(renderContext, renderBatch, handle, pref(Preferences::HandleColor));
  }

  void renderDragHighlight(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
  {
    renderHighlight(renderContext, renderBatch, m_dragHandlePosition);
  }

  void renderDragGuide(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
  {
    renderGuide(renderContext, renderBatch, m_dragHandlePosition);
  }

  template <typename HH>
  void renderHandles(
    const std::vector<HH>& handles,
    Renderer::RenderService& renderService,
    const Color& color) const
  {
    renderService.setForegroundColor(color);
    renderService.renderHandles(kdl::vec_static_cast<typename HH::float_type>(handles));
  }

  template <typename HH>
  void renderHandle(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const HH& handle,
    const Color& color) const
  {
    Renderer::RenderService renderService(renderContext, renderBatch);
    renderService.setForegroundColor(color);
    renderService.renderHandle(typename HH::float_type(handle));
  }

  template <typename HH>
  void renderHighlight(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const HH& handle) const
  {
    Renderer::RenderService renderService(renderContext, renderBatch);
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(typename HH::float_type(handle));
  }

  void renderHighlight(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const vm::vec3& handle) const
  {
    Renderer::RenderService renderService(renderContext, renderBatch);
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(vm::vec3f(handle));

    renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
    renderService.setBackgroundColor(
      pref(Preferences::SelectedInfoOverlayBackgroundColor));
    renderService.renderString(kdl::str_to_string(handle), vm::vec3f(handle));
  }

  template <typename HH>
  void renderGuide(
    Renderer::RenderContext&, Renderer::RenderBatch&, const HH& /* position */) const
  {
  }

  virtual void renderGuide(
    Renderer::RenderContext&,
    Renderer::RenderBatch&,
    const vm::vec3& /* position */) const
  {
  }

protected: // Tool interface
  bool doActivate() override
  {
    m_changeCount = 0;
    connectObservers();

    const std::vector<Model::BrushNode*>& brushes = selectedBrushes();
    handleManager().clear();
    handleManager().addHandles(std::begin(brushes), std::end(brushes));

    return true;
  }

  bool doDeactivate() override
  {
    m_notifierConnection.disconnect();
    handleManager().clear();
    return true;
  }

private: // Observers and state management
  void connectObservers()
  {
    auto document = kdl::mem_lock(m_document);
    m_notifierConnection += document->selectionDidChangeNotifier.connect(
      this, &VertexToolBase::selectionDidChange);
    m_notifierConnection +=
      document->nodesWillChangeNotifier.connect(this, &VertexToolBase::nodesWillChange);
    m_notifierConnection +=
      document->nodesDidChangeNotifier.connect(this, &VertexToolBase::nodesDidChange);
    m_notifierConnection +=
      document->commandDoNotifier.connect(this, &VertexToolBase::commandDo);
    m_notifierConnection +=
      document->commandDoneNotifier.connect(this, &VertexToolBase::commandDone);
    m_notifierConnection +=
      document->commandDoFailedNotifier.connect(this, &VertexToolBase::commandDoFailed);
    m_notifierConnection +=
      document->commandUndoNotifier.connect(this, &VertexToolBase::commandUndo);
    m_notifierConnection +=
      document->commandUndoneNotifier.connect(this, &VertexToolBase::commandUndone);
    m_notifierConnection += document->commandUndoFailedNotifier.connect(
      this, &VertexToolBase::commandUndoFailed);
  }

  void commandDo(Command& command) { commandDoOrUndo(command); }

  void commandDone(Command& command) { commandDoneOrUndoFailed(command); }

  void commandDoFailed(Command& command) { commandDoFailedOrUndone(command); }

  void commandUndo(UndoableCommand& command) { commandDoOrUndo(command); }

  void commandUndone(UndoableCommand& command) { commandDoFailedOrUndone(command); }

  void commandUndoFailed(UndoableCommand& command) { commandDoneOrUndoFailed(command); }

  void commandDoOrUndo(Command& command)
  {
    if (auto* vertexCommand = dynamic_cast<BrushVertexCommandBase*>(&command))
    {
      deselectHandles();
      removeHandles(vertexCommand);
      ++m_ignoreChangeNotifications;
    }
  }

  void commandDoneOrUndoFailed(Command& command)
  {
    if (auto* vertexCommand = dynamic_cast<BrushVertexCommandBase*>(&command))
    {
      addHandles(vertexCommand);
      selectNewHandlePositions(vertexCommand);
      --m_ignoreChangeNotifications;
    }
  }

  void commandDoFailedOrUndone(Command& command)
  {
    if (auto* vertexCommand = dynamic_cast<BrushVertexCommandBase*>(&command))
    {
      addHandles(vertexCommand);
      selectOldHandlePositions(vertexCommand);
      --m_ignoreChangeNotifications;
    }
  }

  void selectionDidChange(const Selection& selection)
  {
    addHandles(selection.selectedNodes());
    removeHandles(selection.deselectedNodes());
  }

  void nodesWillChange(const std::vector<Model::Node*>& nodes)
  {
    if (m_ignoreChangeNotifications == 0u)
    {
      const auto selectedNodes =
        kdl::vec_filter(nodes, [](const auto* node) { return node->selected(); });
      removeHandles(selectedNodes);
    }
  }

  void nodesDidChange(const std::vector<Model::Node*>& nodes)
  {
    if (m_ignoreChangeNotifications == 0u)
    {
      const auto selectedNodes =
        kdl::vec_filter(nodes, [](const auto* node) { return node->selected(); });
      addHandles(selectedNodes);
    }
  }

protected:
  virtual void deselectHandles() { handleManager().deselectAll(); }

  virtual void addHandles(BrushVertexCommandBase* command)
  {
    command->addHandles(handleManager());
  }

  virtual void removeHandles(BrushVertexCommandBase* command)
  {
    command->removeHandles(handleManager());
  }

  virtual void selectNewHandlePositions(BrushVertexCommandBase* command)
  {
    command->selectNewHandlePositions(handleManager());
  }

  virtual void selectOldHandlePositions(BrushVertexCommandBase* command)
  {
    command->selectOldHandlePositions(handleManager());
  }

  template <typename HT>
  void addHandles(
    const std::vector<Model::Node*>& nodes, VertexHandleManagerBaseT<HT>& handleManager)
  {
    for (const auto* node : nodes)
    {
      node->accept(kdl::overload(
        [](const Model::WorldNode*) {},
        [](const Model::LayerNode*) {},
        [](const Model::GroupNode*) {},
        [](const Model::EntityNode*) {},
        [&](const Model::BrushNode* brush) { handleManager.addHandles(brush); },
        [](const Model::PatchNode*) {}));
    }
  }

  template <typename HT>
  void removeHandles(
    const std::vector<Model::Node*>& nodes, VertexHandleManagerBaseT<HT>& handleManager)
  {
    for (const auto* node : nodes)
    {
      node->accept(kdl::overload(
        [](const Model::WorldNode*) {},
        [](const Model::LayerNode*) {},
        [](const Model::GroupNode*) {},
        [](const Model::EntityNode*) {},
        [&](const Model::BrushNode* brush) { handleManager.removeHandles(brush); },
        [](const Model::PatchNode*) {}));
    }
  }

  virtual void addHandles(const std::vector<Model::Node*>& nodes)
  {
    addHandles(nodes, handleManager());
  }

  virtual void removeHandles(const std::vector<Model::Node*>& nodes)
  {
    removeHandles(nodes, handleManager());
  }
};
} // namespace View
} // namespace TrenchBroom
