//
//   Copyright 10/2/20 Ardalan Amini
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


#ifndef SILICON_PLAINOBJECT_H
#define SILICON_PLAINOBJECT_H


#include "Node.h"
#include <map>


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class PlainObject : public Node {
    private:
        map<string, Node *> value;

        explicit PlainObject(map<string, Node *> value);

    public:
        static Node *create(Context *ctx, map<string, Node *>);

        llvm::Value *codegen(Context *ctx) override;

        node_t type() override;

    };

}


#endif //SILICON_PLAINOBJECT_H
