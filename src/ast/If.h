//
//   Copyright 2020 Ardalan Amini
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//


#ifndef SILICON_IF_H
#define SILICON_IF_H


#include <llvm/IR/Instructions.h>
#include "Node.h"


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class If : public Node {
    private:
        Node *condition;
        vector<Node *> then_statements, else_statements;

        bool is_inline = false;

        llvm::Value *inlineCodegen(Context *ctx);

        llvm::Value *conditionCodegen(Context *ctx);

        llvm::Value *thenCodegen(Context *ctx);

        llvm::Value *elseCodegen(Context *ctx);

        bool hasThen();

        bool hasElse();

    public:
        If(const string &location, Node *condition, vector<Node *> then_statements, vector<Node *> else_statements);

        llvm::Value *codegen(Context *ctx) override;

        node_t type() override;

        If *setElse(vector<Node *> statements);

        If *makeInline();

    };

}


#endif //SILICON_IF_H
