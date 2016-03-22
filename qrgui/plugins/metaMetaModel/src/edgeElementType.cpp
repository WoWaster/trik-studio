/* Copyright 2016 Dmitry Mordvinov
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

#include "metaMetaModel/edgeElementType.h"

#include "metaMetaModel/metamodel.h"

using namespace qReal;

EdgeElementType::EdgeElementType(Metamodel &metamodel)
	: ElementType(metamodel)
	, mPenStyle(Qt::NoPen)
	, mPenWidth(0)
	, mIsDividable(true)
	, mShapeType(LinkShape::broken)
{
}

ElementType::Type EdgeElementType::type() const
{
	return Type::edge;
}

Qt::PenStyle EdgeElementType::penStyle() const
{
	return mPenStyle;
}

void EdgeElementType::setPenStyle(Qt::PenStyle style)
{
	mPenStyle = style;
}

int EdgeElementType::penWidth() const
{
	return mPenWidth;
}

void EdgeElementType::setPenWidth(int width)
{
	mPenWidth = width;
}

QColor EdgeElementType::penColor() const
{
	return mPenColor;
}

void EdgeElementType::setPenColor(const QColor &color)
{
	mPenColor = color;
}

bool EdgeElementType::isDividable() const
{
	return mIsDividable;
}

void EdgeElementType::setDividable(bool isDividable)
{
	mIsDividable = isDividable;
}

const QStringList &EdgeElementType::fromPortTypes() const
{
	return mFromPortTypes;
}

void EdgeElementType::setFromPortTypes(const QStringList &portTypes)
{
	mFromPortTypes = portTypes;
}

const QStringList &EdgeElementType::toPortTypes() const
{
	return mToPortTypes;
}

void EdgeElementType::setToPortTypes(const QStringList &portTypes)
{
	mToPortTypes = portTypes;
}

LinkShape EdgeElementType::shapeType() const
{
	return mShapeType;
}

void EdgeElementType::setShapeType(LinkShape shape)
{
	mShapeType = shape;
}

void EdgeElementType::drawStartArrow(QPainter *painter) const
{
	Q_UNUSED(painter)
}

void EdgeElementType::drawEndArrow(QPainter *painter) const
{
	Q_UNUSED(painter)
}
