from sear import sear


def test_valid_traits_user_request():
    """This test is supposed to succeed"""
    valid_traits_result = sear(
            {
            "operation": "get-valid-traits",
            "admin_type": "user",
            },
        )

    assert "errors" not in str(valid_traits_result.result)
    assert valid_traits_result.result["return_codes"]["sear_return_code"] == 0

    valid_traits = valid_traits_result.result["valid_traits"]
    base_traits = valid_traits["base"]
    omvs_traits = valid_traits["omvs"]

    assert "group" not in valid_traits
    assert base_traits["base:group_connections"] == "repeat"
    assert base_traits["base:name"] == "string"
    assert base_traits["base:special"] == "bool"
    assert omvs_traits["omvs:uid"] == "uint"


def test_valid_traits_group_request():
    """This test is supposed to succeed"""
    valid_traits_result = sear(
            {
            "operation": "get-valid-traits",
            "admin_type": "group",
            },
        )

    assert "errors" not in str(valid_traits_result.result)
    assert valid_traits_result.result["return_codes"]["sear_return_code"] == 0

    valid_traits = valid_traits_result.result["valid_traits"]
    base_traits = valid_traits["base"]
    omvs_traits = valid_traits["omvs"]

    assert "user" not in valid_traits
    assert base_traits["base:connected_users"] == "repeat"
    assert base_traits["base:owner"] == "string"
    assert base_traits["base:universal"] == "bool"
    assert omvs_traits["omvs:gid"] == "uint"


def test_valid_traits_dataset_request():
    """This test is supposed to succeed"""
    valid_traits_result = sear(
            {
            "operation": "get-valid-traits",
            "admin_type": "dataset",
            },
        )

    assert "errors" not in str(valid_traits_result.result)
    assert valid_traits_result.result["return_codes"]["sear_return_code"] == 0

    base_traits = valid_traits_result.result["valid_traits"]["base"]

    assert base_traits["base:access_list"] == "repeat"
    assert base_traits["base:access_count"] == "uint"
    assert base_traits["base:dataset_type"] == "string"
    assert base_traits["base:erase_datasets_on_delete"] == "bool"


def test_valid_traits_group_connection_request():
    """This test is supposed to succeed"""
    valid_traits_result = sear(
            {
            "operation": "get-valid-traits",
            "admin_type": "group-connection",
            },
        )

    assert "errors" not in str(valid_traits_result.result)
    assert valid_traits_result.result["return_codes"]["sear_return_code"] == 0

    base_traits = valid_traits_result.result["valid_traits"]["base"]

    assert base_traits["base:automatic_dataset_protection"] == "bool"
    assert base_traits["base:authority"] == "string"
    assert base_traits["base:connection_used_count"] == "uint"


def test_valid_traits_permission_request():
    """This test is supposed to succeed"""
    valid_traits_result = sear(
            {
            "operation": "get-valid-traits",
            "admin_type": "permission",
            },
        )

    assert "errors" not in str(valid_traits_result.result)
    assert valid_traits_result.result["return_codes"]["sear_return_code"] == 0

    base_traits = valid_traits_result.result["valid_traits"]["base"]

    assert base_traits["base:access"] == "string"
    assert base_traits["base:authid"] == "string"
    assert base_traits["base:model_profile_generic"] == "bool"


def test_valid_traits_resource_request():
    """This test is supposed to succeed"""
    valid_traits_result = sear(
            {
            "operation": "get-valid-traits",
            "admin_type": "resource",
            },
        )

    assert "errors" not in str(valid_traits_result.result)
    assert valid_traits_result.result["return_codes"]["sear_return_code"] == 0

    base_traits = valid_traits_result.result["valid_traits"]["base"]

    assert base_traits["base:access_list"] == "repeat"
    assert base_traits["base:access_count"] == "uint"
    assert base_traits["base:application_data"] == "string"
    assert base_traits["base:model_profile_generic"] == "bool"


def test_valid_traits_racf_options_request():
    """This test is supposed to succeed"""
    valid_traits_result = sear(
            {
            "operation": "get-valid-traits",
            "admin_type": "racf-options",
            },
        )

    assert "errors" not in str(valid_traits_result.result)
    assert valid_traits_result.result["return_codes"]["sear_return_code"] == 0

    base_traits = valid_traits_result.result["valid_traits"]["base"]

    assert base_traits["base:audit_classes"] == "repeat"
    assert base_traits["base:password_history"] == "uint"
    assert base_traits["base:uncataloged_dataset_access"] == "string"
    assert base_traits["base:add_creator_to_access_list"] == "bool"