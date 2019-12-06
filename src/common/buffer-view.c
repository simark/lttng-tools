/*
 * Copyright (C) 2017 - Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License, version 2.1 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <common/buffer-view.h>
#include <common/dynamic-buffer.h>
#include <common/error.h>
#include <assert.h>

LTTNG_HIDDEN
struct lttng_buffer_view lttng_buffer_view_init(
		const char *src, size_t offset, ptrdiff_t len)
{
	struct lttng_buffer_view view = { .data = src + offset, .size = len };
	return view;
}

LTTNG_HIDDEN
struct lttng_buffer_view lttng_buffer_view_from_view(
		const struct lttng_buffer_view *src, size_t offset,
		ptrdiff_t len)
{
	struct lttng_buffer_view view = { .data = NULL, .size = 0 };

	assert(src);

	if (offset > src->size) {
		ERR("Attempt to create buffer view with invalid offset");
		goto end;
	}

	if (len != -1 && len > (src->size - offset)) {
		ERR("Attempt to create buffer view with invalid length");
		goto end;
	}

	view.data = src->data + offset;
	view.size = len == -1 ? (src->size - offset) : len;
end:
	return view;
}

LTTNG_HIDDEN
struct lttng_buffer_view lttng_buffer_view_from_dynamic_buffer(
		const struct lttng_dynamic_buffer *src, size_t offset,
		ptrdiff_t len)
{
	struct lttng_buffer_view view = { .data = NULL, .size = 0 };

	assert(src);

	if (offset > src->size) {
		ERR("Attempt to create buffer view with invalid offset");
		goto end;
	}

	if (len != -1 && len > (src->size - offset)) {
		ERR("Attempt to create buffer view with invalid length");
		goto end;
	}

	view.data = src->data + offset;
	view.size = len == -1 ? (src->size - offset) : len;
end:
	return view;
}

LTTNG_HIDDEN
bool lttng_buffer_view_validate_string(const struct lttng_buffer_view *buf,
		const char *str,
		size_t len_with_null_terminator)
{
	const char *past_buf_end;
	size_t max_str_len_with_null_terminator;
	size_t str_len;
	bool ret;

	past_buf_end = buf->data + buf->size;

	/* Is the start of the string in the buffer view? */
	if (str < buf->data || str >= past_buf_end) {
		ret = false;
		goto end;
	}

	/*
	 * Max length the string could have to fit in the buffer, including
	 * NULL terminator.
	 */
	max_str_len_with_null_terminator = past_buf_end - str;

	/* Could the string even fit in the buffer? */
	if (len_with_null_terminator > max_str_len_with_null_terminator) {
		ret = false;
		goto end;
	}

	str_len = lttng_strnlen(str, max_str_len_with_null_terminator);
	if (str_len != (len_with_null_terminator - 1)) {
		ret = false;
		goto end;
	}

	ret = true;

end:
	return ret;
}
