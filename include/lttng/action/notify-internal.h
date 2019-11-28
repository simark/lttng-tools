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

#ifndef LTTNG_ACTION_NOTIFY_INTERNAL_H
#define LTTNG_ACTION_NOTIFY_INTERNAL_H

#include <lttng/action/notify.h>
#include <lttng/action/action-internal.h>

struct lttng_action_notify {
	struct lttng_action parent;
};

LTTNG_HIDDEN
ssize_t lttng_action_notify_create_from_buffer(
		const struct lttng_buffer_view *view,
		struct lttng_action **action);

#endif /* LTTNG_ACTION_NOTIFY_INTERNAL_H */
