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


#ifndef SILICON_FUNCTION_H
#define SILICON_FUNCTION_H


#include <vector>
#include "llvm/IR/Function.h"
#include "Node.h"
#include "Prototype.h"


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class Function : public Node {
    private:
        Prototype *prototype;
        vector<Node *> body;

    public:
        Function(const string &location, Prototype *prototype, vector<Node *> body);

        llvm::Function *codegen(Context *ctx) override;

        node_t type() override;

        Function *externalLinkage();
    };

}


#endif //SILICON_FUNCTION_H
