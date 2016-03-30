/* Copyright 2007-2015 QReal Research Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once

#include <QtCore/QString>
#include <QtCore/QSharedPointer>

#include <qrgui/plugins/toolPluginInterface/usedInterfaces/logicalModelAssistInterface.h>

#include "ast/list.h"

#include "generator/commonInfo/variablesTable.h"
#include "generator/commonInfo/currentScope.h"

namespace generationRules {
namespace generator {

/// Class that generates list of ids for list node.
class ListGenerator
{
public:
	/// Returns list of ids for given list node.
	/// @param listNode - list node.
	/// @param logicalModelInterface - model interface.
	/// @param variablesTable - table of variables.
	/// @param currentScope - information about current scope.
	static qReal::IdList listOfIds(const QSharedPointer<simpleParser::ast::List> &listNode
			, qReal::LogicalModelAssistInterface *logicalModelInterface
			, VariablesTable &variablesTable
			, const CurrentScope &currentScope);
};
}
}
