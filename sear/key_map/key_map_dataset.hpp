#ifndef __SEAR_KEY_MAP_DATASET_H_
#define __SEAR_KEY_MAP_DATASET_H_

#include <stdbool.h>

#include "key_map_structs.hpp"

const trait_key_mapping_t DATASET_BASE_SEGMENT_MAP[]{
    {
     "base:access_list",   "aclcnt",
     TRAIT_TYPE_REPEAT, {false, false, false, false},
     },
    {
     "base:access_count",  "aclacnt",
     TRAIT_TYPE_UINT, {false, false, false, false},
     },
    {
     "base:access_type",   "aclacs",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:access_id",    "aclid",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:conditional_access_list",  "acl2cnt",
     TRAIT_TYPE_REPEAT, {false, false, false, false},
     },
    {
     "base:conditional_access_count", "acl2acnt",
     TRAIT_TYPE_UINT, {false, false, false, false},
     },
    {
     "base:conditional_access_type",  "acl2acs",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:conditional_access_class", "acl2cond",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:conditional_access_entity",  "acl2ent",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:conditional_access_id",   "acl2id",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:alter_access_count",  "acsaltr",
     TRAIT_TYPE_UINT, {false, false, false, false},
     },
    {
     "base:control_access_count",  "acscntl",
     TRAIT_TYPE_UINT, {false, false, false, false},
     },
    {
     "base:read_access_count",  "acsread",
     TRAIT_TYPE_UINT, {false, false, false, false},
     },
    {
     "base:update_access_count",  "acsupdt",
     TRAIT_TYPE_UINT, {false, false, false, false},
     },
    {
     "base:alter_volume",   "altvol",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:audit_alter",  "audaltr",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:audit_control",  "audcntl",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:audit_none",  "audnone",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:audit_read",  "audread",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:audit_update",  "audupdt",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:security_category", "category",
     TRAIT_TYPE_STRING,    {true, true, true, false},
     },
    {
     "base:security_categories",  "numctgy",
     TRAIT_TYPE_REPEAT, {false, false, false, false},
     },
    {
     "base:create_date", "creatdat",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:installation_data",     "data",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:dataset_type",   "dstype",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:erase_datasets_on_delete",    "erase",
     TRAIT_TYPE_BOOLEAN,   {true, false, false, true},
     },
    {
     "base:model_profile_class",   "fclass",
     TRAIT_TYPE_STRING,  {true, false, false, false},
     },
    {
     "base:model_profile_generic", "fgeneric",
        TRAIT_TYPE_BOOLEAN,  {true, false, false, false},
     },
    {
     "base:tape_dataset_file_sequence_number",  "fileseq",
     TRAIT_TYPE_UINT,  {true, false, false, false},
     },
    {
     "base:model_profile",     "from",
     TRAIT_TYPE_STRING,  {true, false, false, false},
     },
    {
     "base:model_profile_volume",  "fvolume",
     TRAIT_TYPE_STRING,  {true, false, false, false},
     },
    {
     "base:global_audit_alter", "gaudaltr",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:global_audit_control", "gaudcntl",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:global_audit_none", "gaudnone",
     TRAIT_TYPE_BOOLEAN,   {true, false, false, true},
     },
    {
     "base:global_audit_read", "gaudread",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:global_audit_update", "gaudupdt",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:high_level_qualifier_is_group",  "groupds",
     TRAIT_TYPE_BOOLEAN, {false, false, false, false},
     },
    {
     "base:creation_group_name",  "groupnm",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:last_change_date",  "lchgdat",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:level",    "level",
     TRAIT_TYPE_UINT,   {true, false, false, true},
     },
    {
     "base:last_reference_date",  "lrefdat",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:dataset_model_profile",    "model",
     TRAIT_TYPE_BOOLEAN,  {true, false, false, false},
     },
    {
     "base:notify_userid",   "notify",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:owner",    "owner",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:auditing",   "raudit",
     TRAIT_TYPE_REPEAT, {false, false, false, false},
     },
    {
     "base:tape_dataset_security_retention_period",    "retpd",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:global_auditing",  "rgaudit",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:security_label", "seclabel",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:security_level", "seclevel",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:statistics", "stats",
     TRAIT_TYPE_BOOLEAN, {false, false, false, false},
     },
    {
     "base:racf_indicated_dataset",      "set",
     TRAIT_TYPE_BOOLEAN,   {true, false, false, true},
     },
    {
     "base:create_only_tape_vtoc_entry",  "setonly",
     TRAIT_TYPE_BOOLEAN,   {true, false, false, false},
     },
    {
     "base:use_tape_dataset_profile",     "tape",
     TRAIT_TYPE_BOOLEAN, {false, false, false, false},
     },
    {
     "base:universal_access",     "uacc",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:dataset_allocation_unit",     "unit",
     TRAIT_TYPE_STRING,   {true, false, false, true},
     },
    {
     "base:volume",   "volume",
     TRAIT_TYPE_STRING,    {true, true, false, true},
     },
    {
     "base:resident_volume",   "volser",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "base:resident_volumes",   "volcnt",
     TRAIT_TYPE_REPEAT, {false, false, false, false},
     },
    {
     "base:warn_on_insufficient_access",  "warning",
     TRAIT_TYPE_BOOLEAN,   {true, false, false, true},
     }
};

const trait_key_mapping_t DATASET_CSDATA_KEY_MAP[]{
    {
     "csdata:*", "*",
     TRAIT_TYPE_STRING, {true, false, false, true},
     }
};

const trait_key_mapping_t DATASET_DFP_KEY_MAP[]{
    {
     "dfp:encryption_type", "enctypes",
     TRAIT_TYPE_STRING, {true, true, true, true},
     },
    {
     "dfp:owner", "resowner",
     TRAIT_TYPE_STRING, {false, false, false, false},
     },
    {
     "dfp:ckds_data_key",  "datakey",
     TRAIT_TYPE_STRING, {false, false, false, false},
     }
};

const trait_key_mapping_t DATASET_TME_KEY_MAP[]{
    {
    "tme:role", "roles",
    TRAIT_TYPE_STRING, {true, true, true, true},
    },
    {
    "tme:roles", "rolen",
    TRAIT_TYPE_REPEAT, {false, false, false, false},
     }
};

const segment_key_mapping_t DATASET_SEGMENT_KEY_MAP[] = {
    {  "base", field_count(DATASET_BASE_SEGMENT_MAP), DATASET_BASE_SEGMENT_MAP},
    {"csdata",   field_count(DATASET_CSDATA_KEY_MAP),   DATASET_CSDATA_KEY_MAP},
    {   "dfp",      field_count(DATASET_DFP_KEY_MAP),      DATASET_DFP_KEY_MAP},
    {   "tme",      field_count(DATASET_TME_KEY_MAP),      DATASET_TME_KEY_MAP}
};

#endif
