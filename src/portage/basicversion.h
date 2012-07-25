// vim:set noet cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_BASICVERSION_H_
#define SRC_PORTAGE_BASICVERSION_H_ 1

#include <config.h>

#include <list>
#include <string>

// include <portage/basicversion.h> make check_includes happy

class BasicPart
{
public:
	typedef signed char SignedBool;
	enum PartType
	{
		garbage,
		alpha,
		beta,
		pre,
		rc,
		revision,
		inter_rev,
		patch,
		character,
		primary,
		first
	};
	// This must be larger than PartType elements and should be a power of 2.
	static const std::string::size_type max_type = 32;
	PartType parttype;
	std::string partcontent;

	BasicPart()
	{ }

	explicit BasicPart(PartType p) : parttype(p), partcontent()
	{ }

	BasicPart(PartType p, std::string s) : parttype(p), partcontent(s)
	{ }

	BasicPart(PartType p, std::string s, std::string::size_type start) : parttype(p), partcontent(s, start)
	{ }

	BasicPart(PartType p, std::string s, std::string::size_type start, std::string::size_type end) : parttype(p), partcontent(s, start, end)
	{ }

	BasicPart(PartType p, char c) : parttype(p), partcontent(1, c)
	{ }

	BasicPart(PartType p, const char *s) : parttype(p), partcontent(s)
	{ }

	static BasicPart::SignedBool compare(const BasicPart& left, const BasicPart& right) ATTRIBUTE_PURE;
};


/** Parse and represent a portage version-string. */
class BasicVersion
{
public:
	enum ParseResult
	{
		parsedOK,
		parsedError,
		parsedGarbage
	};

	virtual ~BasicVersion() { }

	/// Parse the version-string pointed to by str.
	BasicVersion::ParseResult parseVersion(const std::string& str, std::string *errtext, bool accept_garbage = true);

	/// Compare all except gentoo revisions
	static BasicPart::SignedBool compareTilde(const BasicVersion& right, const BasicVersion& left) ATTRIBUTE_PURE;

	/// Compare the version.
	static BasicPart::SignedBool compare(const BasicVersion& right, const BasicVersion& left) ATTRIBUTE_PURE;

	std::string getFull() const;

	std::string getPlain() const;

	std::string getRevision() const;

protected:
	/// Splitted m_primsplit-version.
	std::list<BasicPart> m_parts;
};


// Short compare-stuff
inline bool operator <  (const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline bool
operator <  (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) < 0; }

inline bool operator >  (const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline bool
operator >  (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) > 0; }

inline bool operator == (const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline bool
operator == (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) == 0; }

inline bool operator != (const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline bool
operator != (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) != 0; }

inline bool operator >= (const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline bool
operator >= (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) >= 0; }

inline bool operator <= (const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline bool
operator <= (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) <= 0; }

#endif  // SRC_PORTAGE_BASICVERSION_H_
