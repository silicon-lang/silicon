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


#ifndef SILICON_VARIABLE_H
#define SILICON_VARIABLE_H


#include "Node.h"


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class Variable : public Node {
    private:
        string name;
        Node *context;

        uint64_t element_index(Context *ctx);

    public:
        Variable(const string &location, string name, Node *context = nullptr);

        llvm::Value *codegen(Context *ctx) override;

        node_t type() override;

        string getName();

        llvm::Type *getType(Context *ctx);

        llvm::Value *get_pointer(Context *ctx);

    };

}


#endif //SILICON_VARIABLE_H
