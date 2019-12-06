/*
 * Copyright (C) 2019 - Jonathan Rajotte <jonathan.rajotte-julien@efficios.com>
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

#ifndef LTTNG_CONDITION_event_rule_INTERNAL_H
#define LTTNG_CONDITION_event_rule_INTERNAL_H

#include <lttng/condition/condition-internal.h>
#include <common/buffer-view.h>
#include <common/macros.h>

struct lttng_condition_event_rule {
	struct lttng_condition parent;
	struct lttng_event_rule *rule;
};

struct lttng_condition_event_rule_comm {
	/* length excludes its own length */
	uint32_t length;
	/* rule */
	char payload[];
} LTTNG_PACKED;


LTTNG_HIDDEN
ssize_t lttng_condition_event_rule_create_from_buffer(
		const struct lttng_buffer_view *view,
		struct lttng_condition **condition);

#endif /* LTTNG_CONDITION_event_rule_INTERNAL_H */
