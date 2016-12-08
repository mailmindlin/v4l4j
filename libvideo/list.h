/*
#             (C) 2008 Hans de Goede <hdegoede@redhat.com>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335  USA
*/

#ifndef __LIST_H
#define __LIST_H

#include "log.h"

#define ARRAY_GROW(array, element_type, old_capacity, min_capacity, growth_factor) do {\
			size_t _old_capacity = (old_capacity);\
			size_t _min_capacity = (min_capacity); \
			if (_old_capacity >= _min_capacity)\
				break;\
			size_t new_capacity = (_old_capacity * growth_factor) + 1;\
			if (new_capacity < _min_capacity)\
				new_capacity = _min_capacity;\
			XREALLOC((array), element_type *, new_capacity * sizeof(element_type));\
			if ((array))\
				(old_capacity) = new_capacity;\
		} while (0)

#endif