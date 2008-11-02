// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "exceptions.h"

using namespace std;

const string&
SysError::getMessage() const throw()
{
	if (m_cache.empty()) {
		m_cache = ExBasic::getMessage();
		if (! m_cache.empty())
			m_cache.append(": ");
		m_cache.append(m_error);
	}
	return m_cache;
}

/// Provide a common look for error-messages for parse-errors in
/// portage.{mask,keywords,..}.
void
portage_parse_error(const string &file, const int line_nr, const string& line, const exception &e)
{
	cerr << "-- Invalid line in " << file << "(" << line_nr << "): \""
		<< line << "\"" << endl;

	// Indent the message correctly
	vector<string> lines = split_string(e.what(), "\n", false);
	for(vector<string>::iterator i = lines.begin();
		i != lines.end();
		++i)
	{
		cerr << "    " << *i << endl;
	}
	cerr << endl;
}