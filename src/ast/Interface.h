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


#ifndef SILICON_INTERFACE_H
#define SILICON_INTERFACE_H


#include "Node.h"
#include "Type.h"


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class Interface : public Node {
    private:
        string name;
        vector<pair<string, Type *>> properties;
        vector<string> parents;

    public:
        Interface(const string &location, string name, vector<pair<string, Type *>> properties, vector<string> parents = {});

        llvm::Value *codegen(Context *ctx) override;

        node_t type() override;

        long property_index(Context *ctx, const string &property);

        vector<pair<string, Type *>> get_properties(Context *ctx);

    };

}


#endif //SILICON_INTERFACE_H
