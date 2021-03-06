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


#ifndef SILICON_WHILE_H
#define SILICON_WHILE_H


#include <llvm/IR/Instructions.h>
#include "Node.h"


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class While : public Node {
    private:
        Node *condition;
        vector<Node *> body;

        bool is_do_while = false;

        llvm::Value *conditionCodegen(Context *ctx);

        llvm::Value *bodyCodegen(Context *ctx);

        bool hasBody();

    public:
        While(const string &location, Node *condition, vector<Node *> body);

        llvm::Value *codegen(Context *ctx) override;

        node_t type() override;

        While *makeDoWhile();

    };

}


#endif //SILICON_WHILE_H
