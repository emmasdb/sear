"""Exercise SEAR certificate deletion on z/OS and remove test objects."""

import argparse
import datetime
import json
from pathlib import Path
import secrets
import subprocess
import sys
import tempfile

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.x509.oid import NameOID

from sear import sear


def require_success(result: dict, operation: str) -> None:
    return_codes = result.get("return_codes", {})
    if result.get("errors") or return_codes.get("sear_return_code") != 0:
        raise RuntimeError(
            f"{operation} failed: {json.dumps(result, sort_keys=True)}"
        )


def make_certificate(path: Path) -> None:
    key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "SEAR Test"),
            x509.NameAttribute(NameOID.COMMON_NAME, "SEAR delete test"),
        ]
    )
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    certificate = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(subject)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(now)
        .not_valid_after(now + datetime.timedelta(days=2))
        .sign(key, hashes.SHA256())
    )

    path.write_bytes(certificate.public_bytes(serialization.Encoding.PEM))
    subprocess.run(
        ["chtag", "-tc", "ISO8859-1", str(path)],
        check=True,
    )


def tso_cleanup(command: str) -> bool:
    try:
        completed = subprocess.run(
            ["tsocmd", command],
            check=False,
            capture_output=True,
            text=True,
        )
    except OSError as error:
        print(f"Cleanup command could not run: {error}", file=sys.stderr)
        return False

    if completed.returncode != 0:
        output = (completed.stdout + completed.stderr).strip()
        print(f"Cleanup command failed: {command}\n{output}", file=sys.stderr)
        return False
    return True


def run_test(owner: str) -> None:
    owner_for_request = owner.lower()
    keyring = f"SEARPY{secrets.token_hex(12).upper()}"
    label = f"SEAR{secrets.token_hex(12).upper()}"
    keyring_created = False
    certificate_may_exist = False
    cleanup_errors: list[str] = []

    try:
        ring_result = sear(
            {
                "operation": "add",
                "admin_type": "keyring",
                "owner": owner_for_request,
                "keyring": keyring,
            }
        ).result
        require_success(ring_result, "Create temporary keyring")
        keyring_created = True

        with tempfile.TemporaryDirectory(prefix="sear-cert-delete-") as temp_dir:
            certificate_path = Path(temp_dir) / "certificate.pem"
            make_certificate(certificate_path)

            certificate_may_exist = True
            add_result = sear(
                {
                    "operation": "add",
                    "admin_type": "certificate",
                    "owner": owner_for_request,
                    "keyring_owner": owner.upper(),
                    "keyring": keyring,
                    "label": label,
                    "certificate_file": str(certificate_path),
                    "usage": "personal",
                    "status": "NOTRUST",
                }
            ).result
            require_success(add_result, "Add temporary certificate")

            delete_result = sear(
                {
                    "operation": "delete",
                    "admin_type": "certificate",
                    "owner": owner_for_request,
                    "keyring_owner": owner.upper(),
                    "keyring": keyring,
                    "label": label,
                }
            ).result
            require_success(delete_result, "Delete temporary certificate")
            certificate_may_exist = False

        print(f"Certificate delete passed for temporary keyring {keyring}.")
    finally:
        if certificate_may_exist:
            if not tso_cleanup(
                f"RACDCERT DELETE(LABEL('{label}')) ID({owner.upper()})"
            ):
                cleanup_errors.append("temporary certificate cleanup failed")

        if keyring_created:
            try:
                delete_ring_result = sear(
                    {
                        "operation": "delete",
                        "admin_type": "keyring",
                        "owner": owner_for_request,
                        "keyring": keyring,
                    }
                ).result
                require_success(delete_ring_result, "Delete temporary keyring")
            except Exception as error:
                print(f"SEAR keyring cleanup failed: {error}", file=sys.stderr)
                if not tso_cleanup(
                    f"RACDCERT DELRING({keyring}) ID({owner.upper()})"
                ):
                    cleanup_errors.append("temporary keyring cleanup failed")


    if cleanup_errors:
        raise RuntimeError("; ".join(cleanup_errors))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--owner",
        required=True,
        help="RACF user ID that owns the temporary keyring and certificate",
    )
    args = parser.parse_args()
    owner = args.owner.strip()
    if not 1 <= len(owner) <= 8 or not all(
        char.isalnum() or char in "@#$" for char in owner
    ):
        parser.error("--owner must be a 1-8 character RACF user ID")

    try:
        run_test(owner)
    except Exception as error:
        print(f"Certificate delete test failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())