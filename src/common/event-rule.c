/*
 * Copyright (C) 2019 - Jonathan Rajotte-Julien <jonathan.rajotte-julien@efficios.com>
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

#include <lttng/event-rule/event-rule-internal.h>
#include <lttng/event-rule/event-rule-kprobe-internal.h>
#include <lttng/event-rule/event-rule-kretprobe-internal.h>
#include <lttng/event-rule/event-rule-uprobe-internal.h>
#include <lttng/event-rule/event-rule-syscall-internal.h>
#include <lttng/event-rule/event-rule-tracepoint-internal.h>
#include <common/macros.h>
#include <common/error.h>
#include <common/dynamic-buffer.h>
#include <common/buffer-view.h>
#include <stdbool.h>
#include <assert.h>

enum lttng_event_rule_type lttng_event_rule_get_type(
		const struct lttng_event_rule *event_rule)
{
	return event_rule ? event_rule->type : LTTNG_EVENT_RULE_TYPE_UNKNOWN;
}

void lttng_event_rule_destroy(struct lttng_event_rule *event_rule)
{
	if (!event_rule) {
		return;
	}

	assert(event_rule->destroy);
	event_rule->destroy(event_rule);
}

LTTNG_HIDDEN
bool lttng_event_rule_validate(const struct lttng_event_rule *event_rule)
{
	bool valid;

	if (!event_rule) {
		valid = false;
		goto end;
	}

	if (!event_rule->validate) {
		/* Sub-class guarantees that it can never be invalid. */
		valid = true;
		goto end;
	}

	valid = event_rule->validate(event_rule);
end:
	return valid;
}

LTTNG_HIDDEN
int lttng_event_rule_serialize(const struct lttng_event_rule *event_rule,
		struct lttng_dynamic_buffer *buf)
{
	int ret;
	struct lttng_event_rule_comm event_rule_comm = { 0 };

	if (!event_rule) {
		ret = -1;
		goto end;
	}

	event_rule_comm.event_rule_type = (int8_t) event_rule->type;

	ret = lttng_dynamic_buffer_append(buf, &event_rule_comm,
			sizeof(event_rule_comm));
	if (ret) {
		goto end;
	}

	ret = event_rule->serialize(event_rule, buf);
	if (ret) {
		goto end;
	}
end:
	return ret;
}

LTTNG_HIDDEN
bool lttng_event_rule_is_equal(const struct lttng_event_rule *a,
		const struct lttng_event_rule *b)
{
	bool is_equal = false;

	if (!a || !b) {
		goto end;
	}

	if (a->type != b->type) {
		goto end;
	}

	if (a == b) {
		is_equal = true;
		goto end;
	}

	is_equal = a->equal ? a->equal(a, b) : true;
end:
	return is_equal;
}

LTTNG_HIDDEN
ssize_t lttng_event_rule_create_from_buffer(
		const struct lttng_buffer_view *buffer,
		struct lttng_event_rule **event_rule)
{
	ssize_t ret, event_rule_size = 0;
	const struct lttng_event_rule_comm *event_rule_comm;
	event_rule_create_from_buffer_cb create_from_buffer = NULL;

	if (!buffer || !event_rule) {
		ret = -1;
		goto end;
	}

	DBG("Deserializing event_rule from buffer");
	event_rule_comm = (const struct lttng_event_rule_comm *) buffer->data;
	event_rule_size += sizeof(*event_rule_comm);

	switch ((enum lttng_event_rule_type) event_rule_comm->event_rule_type) {
	case LTTNG_EVENT_RULE_TYPE_TRACEPOINT:
		create_from_buffer = lttng_event_rule_tracepoint_create_from_buffer;
		break;
	case LTTNG_EVENT_RULE_TYPE_KPROBE:
		create_from_buffer = lttng_event_rule_kprobe_create_from_buffer;
		break;
	case LTTNG_EVENT_RULE_TYPE_KRETPROBE:
		create_from_buffer = lttng_event_rule_kretprobe_create_from_buffer;
		break;
	case LTTNG_EVENT_RULE_TYPE_UPROBE:
		create_from_buffer = lttng_event_rule_uprobe_create_from_buffer;
		break;
	case LTTNG_EVENT_RULE_TYPE_SYSCALL:
		create_from_buffer = lttng_event_rule_syscall_create_from_buffer;
		break;
	default:
		ERR("Attempted to create event rule of unknown type (%i)",
				(int) event_rule_comm->event_rule_type);
		ret = -1;
		goto end;
	}

	if (create_from_buffer) {
		const struct lttng_buffer_view view =
				lttng_buffer_view_from_view(buffer,
					sizeof(*event_rule_comm), -1);

		ret = create_from_buffer(&view, event_rule);
		if (ret < 0) {
			goto end;
		}
		event_rule_size += ret;

	} else {
		abort();
	}

	ret = event_rule_size;
end:
	return ret;
}

LTTNG_HIDDEN
void lttng_event_rule_init(struct lttng_event_rule *event_rule,
		enum lttng_event_rule_type type)
{
	event_rule->type = type;
}
