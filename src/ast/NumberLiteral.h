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


#ifndef SILICON_NUMBERLITERAL_H
#define SILICON_NUMBERLITERAL_H


#include "Node.h"


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class NumberLiteral : public Node {
    private:
        string value;

        explicit NumberLiteral(string value);

    public:
        static Node *create(Context *ctx, string value);

        llvm::Value *codegen(Context *ctx) override;

        node_t type() override;

    };

}


#endif //SILICON_NUMBERLITERAL_H
