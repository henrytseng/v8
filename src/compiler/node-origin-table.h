// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_COMPILER_NODE_ORIGIN_TABLE_H_
#define V8_COMPILER_NODE_ORIGIN_TABLE_H_

#include <limits>

#include "src/base/compiler-specific.h"
#include "src/compiler/node-aux-data.h"
#include "src/globals.h"
#include "src/source-position.h"

namespace v8 {
namespace internal {
namespace compiler {

class NodeOrigin {
 public:
  NodeOrigin(const char* reducer_name, NodeId created_from)
      : reducer_name_(reducer_name), created_from_(created_from) {}
  NodeOrigin(const NodeOrigin& other) = default;
  static NodeOrigin Unknown() { return NodeOrigin(); }

  bool IsKnown() { return created_from_ >= 0; }
  int64_t created_from() const { return created_from_; }
  const char* reducer_name() const { return reducer_name_; }

  bool operator==(const NodeOrigin& o) const {
    return reducer_name_ == o.reducer_name_ && created_from_ == o.created_from_;
  }

  void PrintJson(std::ostream& out) const;

 private:
  NodeOrigin()
      : reducer_name_(""), created_from_(std::numeric_limits<int64_t>::min()) {}
  const char* reducer_name_;
  int64_t created_from_;
};

inline bool operator!=(const NodeOrigin& lhs, const NodeOrigin& rhs) {
  return !(lhs == rhs);
}

class V8_EXPORT_PRIVATE NodeOriginTable final
    : public NON_EXPORTED_BASE(ZoneObject) {
 public:
  class Scope final {
   public:
    Scope(NodeOriginTable* origins, const char* reducer_name, Node* node)
        : origins_(origins), prev_origin_(NodeOrigin::Unknown()) {
      if (origins) {
        prev_origin_ = origins->current_origin_;
        origins->current_origin_ = NodeOrigin(reducer_name, node->id());
      }
    }

    ~Scope() {
      if (origins_) origins_->current_origin_ = prev_origin_;
    }

   private:
    NodeOriginTable* const origins_;
    NodeOrigin prev_origin_;
    DISALLOW_COPY_AND_ASSIGN(Scope);
  };

  explicit NodeOriginTable(Graph* graph);

  void AddDecorator();
  void RemoveDecorator();

  NodeOrigin GetNodeOrigin(Node* node) const;
  void SetNodeOrigin(Node* node, const NodeOrigin& no);

  void SetCurrentPosition(const NodeOrigin& no) { current_origin_ = no; }

  void PrintJson(std::ostream& os) const;

 private:
  class Decorator;

  Graph* const graph_;
  Decorator* decorator_;
  NodeOrigin current_origin_;
  NodeAuxData<NodeOrigin, NodeOrigin::Unknown> table_;

  DISALLOW_COPY_AND_ASSIGN(NodeOriginTable);
};

}  // namespace compiler
}  // namespace internal
}  // namespace v8

#endif  // V8_COMPILER_NODE_ORIGIN_TABLE_H_
