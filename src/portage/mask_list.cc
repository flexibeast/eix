// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "mask_list.h"
#include <eixTk/exceptions.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <eixTk/stringutils.h>
#include <eixTk/utils.h>
#include <portage/basicversion.h>
#include <portage/keywords.h>
#include <portage/mask.h>
#include <portage/package.h>

#include <map>
#include <string>
#include <vector>

class Version;

using namespace std;

template <>
bool
MaskList<Mask>::add_file(const char *file, Mask::Type mask_type, bool recursive)
{
	vector<string> lines;
	if(!pushback_lines(file, &lines, false, recursive))
		return false;
	bool added(false);
	for(vector<string>::iterator it(lines.begin()); likely(it != lines.end()); ++it) {
		if(it->empty()) {
			continue;
		}
		Mask m(mask_type);
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->c_str(), &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			portage_parse_error(file, lines.begin(), it, errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			add(m);
			added = true;
		}
	}
	return added;
}

// return true if some masks applied
template <>
bool
MaskList<Mask>::applyMasks(Package *p, Keywords::Redundant check) const
{
	Get *masks(get(p));
	if(masks == NULLPTR) {
		return false;
	}
	bool had_mask(false);
	bool had_unmask(false);
	for(Get::const_iterator it(masks->begin());
		likely(it != masks->end()); ++it) {
		it->checkMask(*p, check);
		switch(it->get_type())
		{
			case Mask::maskMask:
				had_mask = true;
				break;
			case Mask::maskUnmask:
				had_unmask = true;
				break;
			default:
				break;
		}
	}
	delete masks;
	if(!(check & Keywords::RED_MASK)) {
		had_mask = false;
	}
	if(!(check & Keywords::RED_UNMASK)) {
		had_unmask = false;
	}
	if(had_mask || had_unmask) {
		for(Package::iterator i(p->begin());
			likely(i != p->end()); ++i) {
			if(had_mask) {
				if(!i->was_masked())
					i->set_redundant(Keywords::RED_MASK);
			}
			if(had_unmask) {
				if(!i->was_unmasked())
					i->set_redundant(Keywords::RED_UNMASK);
			}
		}
	}
	return true;
}

template<>
void
MaskList<Mask>::applySetMasks(Version *v, const string &set_name) const
{
	Get *masks(get_setname(set_name));
	if(masks == NULLPTR) {
		return;
	}
	for(Get::const_iterator it(masks->begin());
		likely(it != masks->end()); ++it) {
		it->apply(v, false, Keywords::RED_NOTHING);
	}
	delete masks;
}

PreListFilename::PreListFilename(const string &n, const char *label)
{
	filename = n;
	if(label == NULLPTR) {
		know_repo = false;
		return;
	}
	know_repo = true;
	m_repo = label;
}

const char *
PreListFilename::repo() const
{
	if(know_repo) {
		return m_repo.c_str();
	}
	return NULLPTR;
}

bool
PreList::handle_lines(const vector<string> &lines, FilenameIndex file, const bool only_add, LineNumber *num)
{
	bool changed(false);
	LineNumber number((num == NULLPTR) ? 1 : (*num));
	for(vector<string>::const_iterator it(lines.begin());
		likely(it != lines.end()); ++it) {
		if(handle_line(*it, file, number++, only_add)) {
			changed = true;
		}
	}
	if(num != NULLPTR) {
		*num = number;
	}
	return changed;
}

bool
PreList::handle_line(const std::string &line, FilenameIndex file, LineNumber number, bool only_add)
{
	if(line.empty()) {
		return false;
	}
	if(only_add || (line[0] != '-')) {
		return add_line(line,file, number);
	}
	return remove_line(line.c_str() + 1);
}

bool
PreList::add_line(const std::string &line, FilenameIndex file, LineNumber number)
{
	vector<string> l;
	split_string(l, line);
	return add_splitted(l, file, number);
}

bool
PreList::remove_line(const std::string &line)
{
	vector<string> l;
	split_string(l, line);
	return remove_splitted(l);
}

bool
PreList::add_splitted(const std::vector<std::string> &line, FilenameIndex file, LineNumber number)
{
	if(line.empty()) {
		return false;
	}
	map<vector<string>, vector<PreListOrderEntry>::size_type>::iterator it(have.find(line));
	if(it == have.end()) {
		have[line] = order.size();
		order.push_back(PreListOrderEntry(line, file, number));
		return true;
	}
	PreListOrderEntry &e(order[it->second]);
	e.filename_index = file;
	e.linenumber = number;
	if(e.removed) {
		e.removed = false;
		return true;
	}
	else {
		e.locally_double = true;
		return false;
	}
}

bool
PreList::remove_splitted(const std::vector<std::string> &line)
{
	if(line.empty()) {
		return false;
	}
	map<vector<string>, vector<PreListOrderEntry>::size_type>::iterator it(have.find(line));
	if(it == have.end()) {
		return false;
	}
	order[it->second].removed = true;
	return true;
}

void
PreList::finalize()
{
	if(finalized) {
		return;
	}
	finalized = true;
	if(order.empty()) {
		return;
	}

	// We first build a map of the result and
	// set the duplicate names to removed.
	map<string,PreListEntry> result;
	for(vector<PreListOrderEntry>::const_iterator it(order.begin());
		likely(it != order.end()); ++it) {
		if(unlikely(it->removed)) {
			continue;
		}
		PreListOrderEntry::const_iterator curr(it->begin());
		PreListEntry *e;
		map<string, PreListEntry>::iterator r(result.find(*curr));
		if(likely(r == result.end())) {
			e = &(result[*curr]);
		}
		else {
			e = &(r->second);
		}
		e->filename_index = it->filename_index;
		e->linenumber     = it->linenumber;
		e->locally_double = it->locally_double;
		e->name = *curr;
		for(++curr; curr != it->end(); ++curr) {
			e->args.push_back(*curr);
		}
	}

	// Now we sort the result according to the order.
	for(vector<PreListOrderEntry>::const_iterator it(order.begin());
		likely(it != order.end()); ++it) {
		if(likely(!it->removed)) {
			push_back(result[(*it)[0]]);
		}
	}

	order.clear();
	have.clear();
}

void
PreList::initialize(MaskList<Mask> &l, Mask::Type t)
{
	finalize();
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		Mask m(t, repo(it->filename_index));
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->name.c_str(), &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			portage_parse_error(file_name(it->filename_index),
				it->linenumber, it->name + " ...", errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			l.add(m);
		}
	}
	l.finalize();
	clear();
}

void
PreList::initialize(MaskList<KeywordMask> &l, string raised_arch)
{
	finalize();
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		KeywordMask m(repo(it->filename_index));
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->name.c_str(), &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			portage_parse_error(file_name(it->filename_index),
				it->linenumber, it->name + " ...", errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			if(it->args.empty()) {
				m.keywords = raised_arch;
			}
			else {
				join_to_string(m.keywords, it->args);
			}
			m.locally_double = it->locally_double;
			l.add(m);
		}
	}
	l.finalize();
	clear();
}

void
PreList::initialize(MaskList<PKeywordMask> &l)
{
	finalize();
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		PKeywordMask m(repo(it->filename_index));
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->name.c_str(), &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			portage_parse_error(file_name(it->filename_index),
				it->linenumber, it->name + " ...", errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			join_to_string(m.keywords, it->args);
			l.add(m);
		}
	}
	l.finalize();
	clear();
}
