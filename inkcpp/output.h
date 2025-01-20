/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "value.h"
#include "platform.h"
#include "snapshot_impl.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			class string_table;
			class list_table;
			class basic_stream : public snapshot_interface
			{
			protected:
				basic_stream(value*, size_t);
			public:
				// Append data to stream
				void append(const value&);

				// Append data array to stream
				void append(const value*, unsigned int length);

				// Append fixed sized data array to stream
				template<unsigned int N>
				void append(const value in[N])
				{
					append(&in[0], N);
				}

				// Returns the number of data items that will be extracted by the next get
				int queued() const;

				// Peeks the top entry
				const value& peek() const;

				// discards data
				void discard(size_t length);

				// Extract into a data array
				void get(value*, size_t length);

				/** Extract to a newly allocated string
				 * @param string_table place to allocate new string in
				 * @param list_table needed do parse list values to string
				 * @tparam RemoveTail if we should remove a tailing space
				 * @return newly allocated string
				 */
				template<bool RemoveTail = true>
				char* get_alloc(string_table&, list_table&);

#ifdef INK_ENABLE_STL
				// Extract into a string
				std::string get();
#elif defined(INK_ENABLE_UNREAL)
				FString get();
#endif

				// Check if the stream is empty
				bool is_empty() const { return _size == 0; }

				/** Returns the entry index since the type was last seen
				 * @param type type to look for in the output
				 * @return entry index or -1 if the type could not be found
				 */
				int entries_since_type(value_type type) const;

				// Checks if the stream ends with a specific type
				bool ends_with(value_type) const;

				// Checks if the last element when save()'d was this type
				bool saved_ends_with(value_type) const;

				// Checks if there are any elements past the save that
				//  are non-whitespace strings
				bool text_past_save() const;

				// Clears the whole stream
				void clear();

				// Marks strings and lists that are in use
				void mark_used(string_table&, list_table&) const;

				// = Save/Restore
				void save();
				void restore();
				void forget();

				// add lists definitions, needed to print lists
				void set_list_meta(const list_table& lists) {
					_lists_table = &lists;
				}

				char last_char() const {
					return _last_char;
				}

				bool saved() const { return _save != ~0; }

				// snapshot interface
				size_t snap(unsigned char* data, const snapper&) const;
				const unsigned char* snap_load(const unsigned char* data, const loader&);

			private:
				size_t find_start() const;
				bool should_skip(size_t iter, bool& hasGlue, bool& lastNewline) const;

				template<typename T>
				void copy_string(const char* str, size_t& dataIter, T& output);
				
			private:
				char _last_char = '\0';

				// data stream
				value* _data = nullptr;
				size_t _max = 0;

				// size
				size_t _size = 0;

				// save point
				size_t _save = ~0;

				const list_table* _lists_table = nullptr;
			};

#ifdef INK_ENABLE_STL
			std::ostream& operator <<(std::ostream&, basic_stream&);
			basic_stream& operator >>(basic_stream&, std::string&);
#endif

			template<size_t N>
			class stream : public basic_stream
			{
			public:
				stream() : basic_stream(&_buffer[0], N) { }

			private:
				value _buffer[N];
			};
		}
	}
}
