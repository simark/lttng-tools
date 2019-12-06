/*
 * Copyright (C) 2017 - Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License, version 2.1 only,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <lttng/condition/condition-internal.h>
#include <lttng/condition/event-rule-internal.h>
#include <lttng/event-rule/event-rule-internal.h>
#include <common/macros.h>
#include <common/error.h>
#include <assert.h>
#include <stdbool.h>

#define IS_EVENT_RULE_CONDITION(condition) ( \
	lttng_condition_get_type(condition) == LTTNG_CONDITION_TYPE_EVENT_RULE_HIT \
	)

static
bool lttng_condition_event_rule_validate(
		const struct lttng_condition *condition);
static
int lttng_condition_event_rule_serialize(
		const struct lttng_condition *condition,
		struct lttng_dynamic_buffer *buf);
static
bool lttng_condition_event_rule_is_equal(const struct lttng_condition *_a,
		const struct lttng_condition *_b);
static
void lttng_condition_event_rule_destroy(
		struct lttng_condition *condition);


static
bool lttng_condition_event_rule_validate(
		const struct lttng_condition *condition)
{
	bool valid = false;
	struct lttng_condition_event_rule *event_rule;

	if (!condition) {
		goto end;
	}

	event_rule = container_of(condition,
			struct lttng_condition_event_rule, parent);
	if (!event_rule->rule) {
		ERR("Invalid session event_rule condition: a rule must be set.");
		goto end;
	}

	valid = lttng_event_rule_validate(event_rule->rule);
end:
	return valid;
}

static
int lttng_condition_event_rule_serialize(
		const struct lttng_condition *condition,
		struct lttng_dynamic_buffer *buf)
{
	int ret;
	size_t header_offset, size_before_payload;
	struct lttng_condition_event_rule *event_rule;
	struct lttng_condition_event_rule_comm event_rule_comm = { 0 };
	struct lttng_condition_event_rule_comm *header;

	if (!condition || !IS_EVENT_RULE_CONDITION(condition)) {
		ret = -1;
		goto end;
	}

	DBG("Serializing event rule condition");
	event_rule = container_of(condition, struct lttng_condition_event_rule,
			parent);

	header_offset = buf->size;
	ret = lttng_dynamic_buffer_append(buf, &event_rule_comm,
			sizeof(event_rule_comm));
	if (ret) {
		goto end;
	}

	size_before_payload = buf->size;
	ret = lttng_event_rule_serialize(event_rule->rule, buf);
	if (ret) {
		goto end;
	}

	/* Update payload size */
	header = (struct lttng_condition_event_rule_comm *) ((char *) buf->data + header_offset);
	header->length = buf->size - size_before_payload;

end:
	return ret;
}

static
bool lttng_condition_event_rule_is_equal(const struct lttng_condition *_a,
		const struct lttng_condition *_b)
{
	bool is_equal = false;
	struct lttng_condition_event_rule *a, *b;

	a = container_of(_a, struct lttng_condition_event_rule, parent);
	b = container_of(_b, struct lttng_condition_event_rule, parent);

	/* Both session names must be set or both must be unset. */
	if ((a->rule && !b->rule) ||
			(!a->rule && b->rule)) {
		WARN("Comparing session event_rule conditions with uninitialized rule.");
		goto end;
	}

	is_equal = lttng_event_rule_is_equal(a->rule, b->rule);
end:
	return is_equal;
}

static
void lttng_condition_event_rule_destroy(
		struct lttng_condition *condition)
{
	struct lttng_condition_event_rule *event_rule;

	event_rule = container_of(condition,
			struct lttng_condition_event_rule, parent);


	lttng_event_rule_destroy(event_rule->rule);
	free(event_rule);
}

struct lttng_condition *lttng_condition_event_rule_create(
		struct lttng_event_rule *rule)
{
	struct lttng_condition *parent = NULL;
	struct lttng_condition_event_rule *condition = NULL;

	if (!rule) {
		goto end;
	}

	condition = zmalloc(sizeof(struct lttng_condition_event_rule));
	if (!condition) {
		return NULL;
	}

	lttng_condition_init(&condition->parent, LTTNG_CONDITION_TYPE_EVENT_RULE_HIT);
	condition->parent.validate = lttng_condition_event_rule_validate,
	condition->parent.serialize = lttng_condition_event_rule_serialize,
	condition->parent.equal = lttng_condition_event_rule_is_equal,
	condition->parent.destroy = lttng_condition_event_rule_destroy,

	condition->rule = rule;
	parent = &condition->parent;
end:
	return parent;
}

LTTNG_HIDDEN
ssize_t lttng_condition_event_rule_create_from_buffer(
		const struct lttng_buffer_view *view,
		struct lttng_condition **_condition)
{
	ssize_t offset = 0, event_rule_size;
	const struct lttng_condition_event_rule_comm *comm;
	struct lttng_condition *condition = NULL;
	struct lttng_event_rule *event_rule = NULL;
	struct lttng_buffer_view event_rule_view;

	if (!view || !_condition) {
		offset = -1;
		goto end;
	}

	if (view->size < sizeof(*comm)) {
		ERR("Failed to initialize from malformed event rule condition: buffer too short to contain header");
		offset = -1;
		goto end;
	}

	comm = (const struct lttng_condition_event_rule_comm *) view->data;
	offset += sizeof(*comm);

	/* Struct lttng_event_rule */
	event_rule_view = lttng_buffer_view_from_view(view, offset, -1);
	event_rule_size = lttng_event_rule_create_from_buffer(&event_rule_view, &event_rule);
	if (event_rule_size < 0 || !event_rule) {
		goto end;
	}

	if ((size_t) comm->length != event_rule_size) {
		offset = -1;
		goto end;
	}
	
	/* Move to the end */
	offset += comm->length;

	condition = lttng_condition_event_rule_create(event_rule);
	if (!condition) {
		offset = -1;
		goto end;
	}

	/* Ownership passed on condition event rule create */
	event_rule = NULL;

	*_condition = condition;
	condition = NULL;
end:
	lttng_event_rule_destroy(event_rule);
	lttng_condition_destroy(condition);
	return offset;
}

enum lttng_condition_status
lttng_condition_event_rule_get_rule(
		const struct lttng_condition *condition,
		const struct lttng_event_rule **rule)
{
	struct lttng_condition_event_rule *event_rule;
	enum lttng_condition_status status = LTTNG_CONDITION_STATUS_OK;

	if (!condition || !IS_EVENT_RULE_CONDITION(condition) || !rule) {
		status = LTTNG_CONDITION_STATUS_INVALID;
		goto end;
	}

	event_rule = container_of(condition, struct lttng_condition_event_rule,
			parent);
	if (!event_rule->rule) {
		status = LTTNG_CONDITION_STATUS_UNSET;
		goto end;
	}
	*rule = event_rule->rule;
end:
	return status;
}
