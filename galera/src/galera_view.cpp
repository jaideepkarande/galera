//
// Copyright (C) 2015 Codership Oy <info@codership.com>
//


#include "galera_view.hpp"

#include <algorithm>
#ifdef PXC
#include "gcs.hpp"  // GCS_ARBITRATOR_NAME
#include <cstring>
#endif /* PXC */

galera::View::View()
    :
    members_()
{ }

#ifdef PXC
/* Arbitrator (garbd) joins with an empty incoming address and, by default,
   with GCS_ARBITRATOR_NAME. It never applies write sets, so it never sends
   NBO end and must not be waited for by the NBO end barrier. Both conditions
   are required, so that a data node without incoming address is still
   waited for. A data node named GCS_ARBITRATOR_NAME and without incoming
   address would be ignored as well. */
static bool is_arbitrator(const wsrep_member_info_t& member)
{
    return (member.incoming[0] == '\0' &&
            ::strcmp(member.name, GCS_ARBITRATOR_NAME) == 0);
}
#endif /* PXC */

galera::View::View(const wsrep_view_info_t& view_info)
    :
    members_()
{
    for (int i(0); i < view_info.memb_num; ++i)
    {
#ifdef PXC
        if (is_arbitrator(view_info.members[i])) continue;
#endif /* PXC */
        members_.insert(view_info.members[i].id);
    }
#ifdef PXC
    /* Should never happen, but if every member looks like an arbitrator,
       fall back to waiting for all of them. */
    if (members_.empty())
    {
        for (int i(0); i < view_info.memb_num; ++i)
        {
            members_.insert(view_info.members[i].id);
        }
    }
#endif /* PXC */
}

galera::View::~View()
{ }


bool galera::View::subset_of(const MembSet& mset) const
{
    return std::includes(mset.begin(), mset.end(),
                         members_.begin(), members_.end(), UUIDCmp());
}
