#include "constraintsParser.h"

#include <QtCore/QUuid>
#include <QtXml/QDomDocument>

#include "event.h"

using namespace twoDModel::constraints::details;

ConstraintsParser::ConstraintsParser(Events &events
		, Variables &variables
		, const Objects &objects
		, const utils::TimelineInterface &timeline)
	: mEvents(events)
	, mVariables(variables)
	, mObjects(objects)
	, mTimeline(timeline)
	, mTriggers(mEvents, mVariables)
	, mConditions(mEvents, mVariables, mObjects)
	, mValues(mVariables, mObjects)
{
}

QStringList ConstraintsParser::errors() const
{
	return mErrors;
}

bool ConstraintsParser::parse(const QString &constraintsXml)
{
	mErrors.clear();

	if (constraintsXml.isEmpty()) {
		return true;
	}

	QDomDocument document;
	QString errorMessage;
	int errorLine, errorColumn;
	if (!document.setContent(constraintsXml, &errorMessage, &errorLine, &errorColumn)) {
		mErrors << QString("%1:%2: %3").arg(QString::number(errorLine), QString::number(errorColumn), errorMessage);
		return false;
	}

	if (document.documentElement().tagName().toLower() != "constraints") {
		mErrors << QObject::tr("Root element must be \"constraints\" tag");
		return false;
	}

	return parseConstraints(document.documentElement());
}

bool ConstraintsParser::parseConstraints(const QDomElement &constraints)
{
	int timelimitTagsCount = 0;
	for (QDomElement constraint = constraints.firstChildElement()
			; !constraint.isNull()
			; constraint = constraint.nextSiblingElement())
	{
		if (!addToEvents(parseConstraint(constraint))) {
			return false;
		}

		if (constraint.tagName().toLower() == "timelimit") {
			++timelimitTagsCount;
		}
	}

	if (timelimitTagsCount == 0) {
		error(QObject::tr("There must be a \"timelimit\" constraint."));
		return false;
	}

	if (timelimitTagsCount > 1) {
		error(QObject::tr("There must be only one \"timelimit\" tag."));
		return false;
	}

	return mErrors.isEmpty();
}

Event *ConstraintsParser::parseConstraint(const QDomElement &constraint)
{
	const QString name = constraint.tagName().toLower();

	if (name == "event") {
		return parseEventTag(constraint);
	}

	if (name == "constraint") {
		return parseConstraintTag(constraint);
	}

	if (name == "timelimit") {
		return parseTimeLimitTag(constraint);
	}

	/// @todo: Display unknown tag errors
	return nullptr;
}

Event *ConstraintsParser::parseEventTag(const QDomElement &element)
{
	if (!assertChildrenExactly(element, 2)) {
		return nullptr;
	}

	const QDomElement child1 = element.firstChildElement();
	const QDomElement child2 = child1.nextSiblingElement();
	const QString child1Name = child1.tagName().toLower();
	const QString child2Name = child2.tagName().toLower();

	const bool firstIsCondition = child1Name == "condition" || child1Name == "conditions";
	const QDomElement conditionTag = firstIsCondition ? child1 : child2;
	const QDomElement triggerTag = firstIsCondition ? child2 : child1;
	const QString conditionName = firstIsCondition ? child1Name : child2Name;
	const QString triggerName = firstIsCondition ? child2Name : child1Name;

	if (conditionName != "condition" && conditionName != "conditions") {
		error(QObject::tr("Event tag must have \"condition\" or \"conditions\" child tag. \"%1\" found instead.")
				.arg(conditionTag.tagName()));
		return nullptr;
	}

	if (triggerName != "trigger" && triggerName != "triggers") {
		error(QObject::tr("Event tag must have \"trigger\" or \"triggers\" child tag. \"%1\" found instead.")
				.arg(triggerTag.tagName()));
		return nullptr;
	}

	const bool setUpInitially = boolAttribute(element, "settedUpInitially", false);
	const bool dropsOnFire = boolAttribute(element, "dropsOnFire", true);

	const Trigger trigger = parseTriggersAlternative(triggerTag);

	Event * const result = new Event(id(element), mConditions.constant(true), trigger, dropsOnFire);

	const Condition condition = conditionName == "condition"
			? parseConditionTag(conditionTag, *result)
			: parseConditionsTag(conditionTag, *result);

	result->setCondition(condition);
	if (setUpInitially) {
		result->setUp();
	}

	return result;
}

Event *ConstraintsParser::parseConstraintTag(const QDomElement &element)
{
	// Constraint is just an event with fail trigger.
	// Check-once constraint is an event with 0 timeout forcing to drop.

	if (!assertChildrenExactly(element, 1)) {
		return nullptr;
	}

	if (!assertAttributeNonEmpty(element, "failMessage")) {
		return nullptr;
	}

	const QString failMessage = element.attribute("failMessage");
	const Trigger trigger = mTriggers.fail(failMessage);

	const QString checkOneAttribute = element.attribute("checkOnce", "false").toLower();
	const bool checkOnce = checkOneAttribute == "true";

	Event * const result = new Event(id(element), mConditions.constant(true), trigger);

	Condition condition = parseConditionsAlternative(element.firstChildElement(), *result);

	if (checkOnce) {
		const Value timestamp = mValues.timestamp(mTimeline);
		const Condition timeout = mConditions.timerCondition(0, true, timestamp, *result);
		condition = mConditions.combined({ timeout, condition }, Glue::And);
	}

	result->setCondition(mConditions.negation(condition));
	result->setUp();
	return result;
}

Event *ConstraintsParser::parseTimeLimitTag(const QDomElement &element)
{
	if (!assertHasAttribute(element, "value")) {
		return nullptr;
	}

	// Timelimit is just an event with timeout and fail trigger.
	const int value = intAttribute(element, "value");
	if (value < 0) {
		return nullptr;
	}

	const QString timeLimitMessage = QObject::tr("Program worked for too long time");
	const Value timestamp = mValues.timestamp(mTimeline);
	Event * const event = new Event(id(element)
			, mConditions.constant(true)
			, mTriggers.fail(timeLimitMessage)
			, true);

	const Condition condition = mConditions.timerCondition(value, true, timestamp, *event);
	event->setCondition(condition);
	event->setUp();

	return event;
}

Condition ConstraintsParser::parseConditionsAlternative(const QDomElement &element, Event &event)
{
	const QString name = element.tagName().toLower();
	return name == "conditions"
			? parseConditionsTag(element, event)
			: name == "condition"
					? parseConditionTag(element, event)
					: parseConditionContents(element, event);
}

Condition ConstraintsParser::parseConditionsTag(const QDomElement &element, Event &event)
{
	if (!assertChildrenMoreThan(element, 0) || !assertAttributeNonEmpty(element, "glue")) {
		return mConditions.constant(true);
	}

	const QString glueAttribute = element.attribute("glue").toLower();
	Glue glue;
	if (glueAttribute == "and") {
		glue = Glue::And;
	} else if (glueAttribute == "or") {
		glue = Glue::Or;
	} else {
		error(QObject::tr("\"Glue\" attribute must have value \"and\" or \"or\"."));
		return mConditions.constant(true);
	}

	QList<Condition> conditions;
	for (QDomElement condition = element.firstChildElement()
			; !condition.isNull()
			; condition = condition.nextSiblingElement())
	{
		conditions << parseConditionsAlternative(condition, event);
	}

	return mConditions.combined(conditions, glue);
}

Condition ConstraintsParser::parseConditionTag(const QDomElement &element, Event &event)
{
	if (!assertChildrenExactly(element, 1)) {
		return mConditions.constant(true);
	}

	return parseConditionContents(element.firstChildElement(), event);
}

Condition ConstraintsParser::parseConditionContents(const QDomElement &element, Event &event)
{
	const QString tag = element.tagName().toLower();

	if (tag == "not") {
		return parseNegationTag(element, event);
	}

	if (tag == "equals" || tag.startsWith("notequal")
			|| tag == "greater" || tag == "less"
			|| tag == "notgreater" || tag == "notless")
	{
		return parseComparisonTag(element);
	}

	if (tag == "inside") {
		return parseInsideTag(element);
	}

	if (tag == "settedup" || tag == "dropped") {
		return parseEventSettedDroppedTag(element);
	}

	if (tag == "timer") {
		return parseTimerTag(element, event);
	}

	error(QObject::tr("Unknown tag \"%1\".").arg(element.tagName()));
	return mConditions.constant(true);
}

Condition ConstraintsParser::parseNegationTag(const QDomElement &element, Event &event)
{
	if (!assertChildrenExactly(element, 1)) {
		return mConditions.constant(true);
	}

	return parseConditionsAlternative(element.firstChildElement(), event);
}

Condition ConstraintsParser::parseComparisonTag(const QDomElement &element)
{
	if (!assertChildrenExactly(element, 2)) {
		return mConditions.constant(true);
	}

	const QString operation = element.tagName().toLower();

	const Value leftValue = parseValue(element.firstChildElement());
	const Value rightValue = parseValue(element.firstChildElement().nextSiblingElement());

	if (operation == "equals") {
		return mConditions.equals(leftValue, rightValue);
	}

	if (operation.startsWith("notequal")) {
		return mConditions.notEqual(leftValue, rightValue);
	}

	if (operation == "greater") {
		return mConditions.greater(leftValue, rightValue);
	}

	if (operation == "less") {
		return mConditions.less(leftValue, rightValue);
	}

	if (operation == "notgreater") {
		return mConditions.notGreater(leftValue, rightValue);
	}

	return mConditions.notLess(leftValue, rightValue);
}

Condition ConstraintsParser::parseInsideTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "objectId") || !assertAttributeNonEmpty(element, "regionId")) {
		return mConditions.constant(true);
	}

	return mConditions.inside(element.attribute("objectId"), element.attribute("regionId"));
}

Condition ConstraintsParser::parseEventSettedDroppedTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "id")) {
		return mConditions.constant(true);
	}

	const QString id = element.attribute("id");
	return element.tagName().toLower() == "settedup"
			? mConditions.settedUp(id)
			: mConditions.dropped(id);
}

Condition ConstraintsParser::parseTimerTag(const QDomElement &element, Event &event)
{
	if (!assertAttributeNonEmpty(element, "timeout")) {
		return mConditions.constant(true);
	}

	const int timeout = intAttribute(element, "timeout", 0);
	const bool forceDrop = boolAttribute(element, "forceDropOnTimeout", true);
	const Value timestamp = mValues.timestamp(mTimeline);
	return mConditions.timerCondition(timeout, forceDrop, timestamp, event);
}

Trigger ConstraintsParser::parseTriggersAlternative(const QDomElement &element)
{
	const QString name = element.tagName().toLower();
	return name == "triggers"
			? parseTriggersTag(element)
			: name == "trigger"
					? parseTriggerTag(element)
					: parseTriggerContents(element);
}

Trigger ConstraintsParser::parseTriggersTag(const QDomElement &element)
{
	if (!assertChildrenMoreThan(element, 0)) {
		return mTriggers.doNothing();
	}

	QList<Trigger> triggers;
	for (QDomElement trigger = element.firstChildElement()
			; !trigger.isNull()
			; trigger = trigger.nextSiblingElement())
	{
		triggers << parseTriggersAlternative(trigger);
	}

	return mTriggers.combined(triggers);
}

Trigger ConstraintsParser::parseTriggerTag(const QDomElement &element)
{
	if (!assertChildrenExactly(element, 1)) {
		return mTriggers.doNothing();
	}

	return parseTriggerContents(element.firstChildElement());
}

Trigger ConstraintsParser::parseTriggerContents(const QDomElement &element)
{
	const QString tag = element.tagName().toLower();

	if (tag == "fail") {
		return parseFailTag(element);
	}

	if (tag == "success") {
		return parseSuccessTag(element);
	}

	if (tag == "setvariable") {
		return parseSetVariableTag(element);
	}

	if (tag == "addtovariable") {
		return parseAddToVariableTag(element);
	}

	if (tag == "setup" || tag == "drop") {
		return parseEventSetDropTag(element);
	}

	error(QObject::tr("Unknown tag \"%1\".").arg(element.tagName()));
	return mTriggers.doNothing();
}

Trigger ConstraintsParser::parseFailTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "message")) {
		return mTriggers.doNothing();
	}

	return mTriggers.fail(element.attribute("message"));
}

Trigger ConstraintsParser::parseSuccessTag(const QDomElement &element)
{
	Q_UNUSED(element)
	return mTriggers.success();
}

Trigger ConstraintsParser::parseSetVariableTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "name") || !assertHasAttribute(element, "value")) {
		return mTriggers.doNothing();
	}

	const QString name = element.attribute("name");
	const QVariant value = bestVariant(element.attribute("value"));

	return mTriggers.setVariable(name, value);
}

Trigger ConstraintsParser::parseAddToVariableTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "name") || !assertHasAttribute(element, "value")) {
		return mTriggers.doNothing();
	}

	const QString name = element.attribute("name");
	const QVariant value = bestVariant(element.attribute("value"));

	return mTriggers.addToVariable(name, value);
}

Trigger ConstraintsParser::parseEventSetDropTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "id")) {
		return mTriggers.doNothing();
	}

	const QString id = element.attribute("id");
	return element.tagName().toLower() == "setup"
			? mTriggers.setUpEvent(id)
			: mTriggers.dropEvent(id);
}

Value ConstraintsParser::parseValue(const QDomElement &element)
{
	const QString tag = element.tagName().toLower();

	if (tag == "int") {
		return parseIntTag(element);
	}

	if (tag == "double") {
		return parseDoubleTag(element);
	}

	if (tag == "string") {
		return parseStringTag(element);
	}

	if (tag == "variablevalue") {
		return parseVariableValueTag(element);
	}

	if (tag == "typeof") {
		return parseTypeOfTag(element);
	}

	if (tag == "objectstate") {
		return parseObjectStateTag(element);
	}

	error(QObject::tr("Unknown value \"%1\".").arg(element.tagName()));
	return mValues.invalidValue();
}

Value ConstraintsParser::parseIntTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "value")) {
		return mValues.invalidValue();
	}

	return mValues.intValue(intAttribute(element, "value"));
}

Value ConstraintsParser::parseDoubleTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "value")) {
		return mValues.invalidValue();
	}

	return mValues.doubleValue(doubleAttribute(element, "value"));
}

Value ConstraintsParser::parseStringTag(const QDomElement &element)
{
	if (!assertHasAttribute(element, "value")) {
		return mValues.invalidValue();
	}

	return mValues.stringValue(element.attribute("value"));
}

Value ConstraintsParser::parseVariableValueTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "name")) {
		return mValues.invalidValue();
	}

	return mValues.variableValue(element.attribute("name"));
}

Value ConstraintsParser::parseTypeOfTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "objectId")) {
		return mValues.invalidValue();
	}

	return mValues.typeOf(element.attribute("objectId"));
}

Value ConstraintsParser::parseObjectStateTag(const QDomElement &element)
{
	if (!assertAttributeNonEmpty(element, "objectId") || !assertAttributeNonEmpty(element, "property")) {
		return mValues.invalidValue();
	}

	return mValues.objectState(element.attribute("objectId"), element.attribute("property"));
}

QString ConstraintsParser::id(const QDomElement &element) const
{
	const QString attribute = element.attribute("id");
	const QString id = attribute.isEmpty() ? QUuid::createUuid().toString() : attribute;
	return id;
}

int ConstraintsParser::intAttribute(const QDomElement &element, const QString &attributeName, int defaultValue)
{
	QString const attributeValue = element.attribute(attributeName);
	bool ok = false;
	const int result = attributeValue.toInt(&ok);
	if (!ok) {
		/// @todo: Make it warning
		error(QObject::tr("Invalid integer value \"%1\"").arg(attributeValue));
		return defaultValue;
	}

	return result;
}

qreal ConstraintsParser::doubleAttribute(const QDomElement &element, const QString &attributeName, qreal defaultValue)
{
	QString const attributeValue = element.attribute(attributeName);
	bool ok = false;
	const qreal result = attributeValue.toDouble(&ok);
	if (!ok) {
		/// @todo: Make it warning
		error(QObject::tr("Invalid floating point value \"%1\"").arg(attributeValue));
		return defaultValue;
	}

	return result;
}

bool ConstraintsParser::boolAttribute(const QDomElement &element, const QString &attributeName, bool defaultValue)
{
	const QString defaultString = defaultValue ? "true" : "false";
	const QString stringValue = element.attribute(attributeName, defaultString).toLower();
	if (stringValue != "true" && stringValue != "false") {
		/// @todo: Make it warning
		error(QObject::tr("Invalid boolean value \"%1\" (expected \"true\" or \"false\")")
				.arg(element.attribute(attributeName)));
		return defaultValue;
	}

	return stringValue == "true";
}

QVariant ConstraintsParser::bestVariant(const QString &value) const
{
	bool ok;

	const int intValue = value.toInt(&ok);
	if (ok) {
		return intValue;
	}

	const qreal doubleValue = value.toDouble(&ok);
	if (ok) {
		return doubleValue;
	}

	return value;
}

bool ConstraintsParser::addToEvents(Event * const event)
{
	if (!event) {
		return false;
	}

	const QString id = event->id();
	if (mEvents.contains(id)) {
		return error(QObject::tr("Duplicate id: \"%1\"").arg(id));
	}

	mEvents[event->id()] = event;
	return true;
}

bool ConstraintsParser::assertChildrenExactly(const QDomElement &element, int count)
{
	if (element.childNodes().count() != count) {
		return error(QObject::tr("%1 tag must have exactly %2 child tag(s)")
				.arg(element.tagName(), QString::number(count)));
	}

	return true;
}

bool ConstraintsParser::assertChildrenMoreThan(const QDomElement &element, int count)
{
	if (element.childNodes().count() <= count) {
		return error(QObject::tr("%1 tag must have at least %2 child tag(s)")
				.arg(element.tagName(), QString::number(count + 1)));
	}

	return true;
}

bool ConstraintsParser::assertHasAttribute(const QDomElement &element, const QString &attribute)
{
	if (!element.hasAttribute(attribute)) {
		error(QObject::tr("\"%1\" tag must have \"%2\" attribute.").arg(element.tagName(), attribute));
		return false;
	}

	return true;
}

bool ConstraintsParser::assertTagName(const QDomElement &element, const QString &nameInLowerCase)
{
	if (element.tagName().toLower() != nameInLowerCase) {
		error(QObject::tr("Expected \"%1\" tag, got \"%2\".").arg(nameInLowerCase, element.tagName()));
		return false;
	}

	return true;
}

bool ConstraintsParser::assertAttributeNonEmpty(const QDomElement &element, const QString &attribute)
{
	if (!assertHasAttribute(element, attribute)) {
		return false;
	}

	if (element.attribute(attribute).isEmpty()) {
		error(QObject::tr("Attribute \"%1\" of the tag \"%2\" must not be empty.").arg(element.tagName(), attribute));
		return false;
	}

	return true;
}

bool ConstraintsParser::error(const QString &message)
{
	mErrors << message;
	return false;
}
