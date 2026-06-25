/*
 * Copyright (C) 2022-2025 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <jsoncpp/json/json.h>

#include "src/iutils/Utils.h"

namespace icamera {

/* Rule of Three: All special functions should be defined defined */
/* Destructor: Explicitly declared as virtual ~JsonParserBase() = default */
/* Copy constructor: Deleted by DISALLOW_COPY_AND_ASSIGN macro expansion in Utils.h */
/* Copy assignment: Deleted by DISALLOW_COPY_AND_ASSIGN macro expansion in Utils.h */
/* The macro expands to: TypeName(const TypeName&) = delete; */
/*                       TypeName& operator=(const TypeName&) = delete; */
/* All three (destructor, copy constructor, copy assignment) are defined */
/* Waive by deviation: False Positive */
//coverity[rule_of_three_violation : FALSE] */
class JsonParserBase {
 public:
    JsonParserBase() = default;
    virtual ~JsonParserBase() = default;

    virtual bool run(const std::string& filename) = 0;
 protected:
    Json::Value openJsonFile(const std::string& filename);

 private:
    DISALLOW_COPY_AND_ASSIGN(JsonParserBase);
};

}  // namespace icamera
