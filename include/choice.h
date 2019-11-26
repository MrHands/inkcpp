#pragma once
#include "system.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			class basic_stream;
			class runner_impl;
		}

		/**
		 * An ink choice that is being presented to the user
		 *
		 * Contains all the data about a single choice being faced
		 * by an ink runner. Of primary concern is the index and the
		 * text.
		 *
		 * @see runner
		*/
		class choice
		{
		public:
			/**
			 * Choice index
			 *
			 * Pass this to the runner to choose this choice and
			 * have it follow its branch.
			 *
			 * @returns index of the choice. 0 is the first, etc.
			*/
			int index() const { return _index; }

			/**
			 * Choice text
			 *
			 * Text to display to the user for choosing this choice.
			 *
			 * @returns choice text as a string
			 */
			const char* text() const { return _text; }
		private:
			friend class internal::runner_impl;

			uint32_t path() const { return _path; }
			void setup(internal::basic_stream&, int index, uint32_t path);
		private:
			const char* _text;
			int _index;
			uint32_t _path;
		};
	}
}
