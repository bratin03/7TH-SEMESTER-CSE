# ###########################################################
# /**
#  * Student Information
#  *
#  * Bratin Mondal - 21CS10016
#  * Abir Roy - 21CS30074
#  *
#  * Department of Computer Science and Engineering
#  * Indian Institute of Technology, Kharagpur
#  */
# ###########################################################

# Import necessary libraries
import asyncio
import json
import time
from indy import pool, wallet, did, ledger, anoncreds
from indy.error import IndyError, ErrorCode
from os.path import dirname
from indy import blob_storage
import subprocess


async def verifier_get_entities_from_ledger(
    pool_handle, _did, identifiers, actor, timestamp=None
):
    """
    Retrieve schemas, credential definitions, revocation registry definitions,
    and revocation registries from the ledger for the provided identifiers.

    Parameters:
    - pool_handle: Handle to the ledger connection.
    - _did: Decentralized Identifier used for accessing the ledger.
    - identifiers: List of dictionaries containing schema_id, cred_def_id,
                   and optionally rev_reg_id and rev_reg_seq_no for each entity to retrieve.
    - actor: Identifier for the entity performing the retrieval, used in log messages.
    - timestamp (optional): Timestamp for fetching specific revocation registry entries.

    Returns:
    - Tuple containing JSON strings of the following:
      - retrieved_schemas: Dictionary of schema data retrieved from the ledger.
      - retrieved_cred_defs: Dictionary of credential definitions retrieved from the ledger.
      - revoc_reg_def: Dictionary of revocation registry definitions retrieved from the ledger.
      - revoc_reg_registry: Dictionary of revocation registries retrieved from the ledger.
    """

    # Initialize dictionaries to store retrieved entities
    retrieved_schemas = {}
    retrieved_cred_defs = {}
    revoc_reg_def = {}
    revoc_reg_registry = {}

    # Iterate over each item in the 'identifiers' list
    for item in identifiers:
        # Print a message indicating that we are getting a schema from the ledger
        print('" {} " -> Get Schema from Ledger'.format(actor))

        # Retrieve a schema from the ledger and store it in the 'retrieved_schemas' dictionary
        (received_schema_id, received_schema) = await get_schema(
            pool_handle, _did, item["schema_id"]
        )
        retrieved_schemas[received_schema_id] = json.loads(received_schema)

        # Print a message indicating that we are getting a credential definition from the ledger
        print('" {} " -> Get Credential Definition from Ledger'.format(actor))

        # Retrieve a credential definition from the ledger and store it in the 'retrieved_cred_defs' dictionary
        (received_cred_def_id, received_cred_def) = await get_cred_def(
            pool_handle, _did, item["cred_def_id"]
        )
        retrieved_cred_defs[received_cred_def_id] = json.loads(received_cred_def)

        # Check if the item includes information about revocation
        if "rev_reg_seq_no" in item and item["rev_reg_seq_no"] is not None:
            # Print a message indicating that we are getting a revocation registry definition from the ledger
            print(
                '" {} " -> Get Revocation Registry Definition from Ledger'.format(actor)
            )

            # Build a request to get the revocation registry definition
            get_revoc_reg_def_request = await ledger.build_get_revoc_reg_def_request(
                _did, item["rev_reg_id"]
            )

            # Ensure that the previous request is applied and the data is not None
            get_revoc_reg_def_response = await ensure_previous_request_applied(
                pool_handle,
                get_revoc_reg_def_request,
                lambda response: response["result"]["data"] is not None,
            )

            # Parse the response to get the revocation registry definition and store it in 'revoc_reg_def' dictionary
            (rev_reg_id, revoc_reg_def_json) = (
                await ledger.parse_get_revoc_reg_def_response(
                    get_revoc_reg_def_response
                )
            )

            # Print a message indicating that we are getting a revocation registry from the ledger
            print('" {} " -> Get Revocation Registry from Ledger'.format(actor))

            # If 'timestamp' is not provided, use the timestamp from the item
            if not timestamp:
                timestamp = item["timestamp"]

            # Build a request to get the revocation registry
            get_revoc_reg_request = await ledger.build_get_revoc_reg_request(
                _did, item["rev_reg_id"], timestamp
            )

            # Ensure that the previous request is applied and the data is not None
            get_revoc_reg_response = await ensure_previous_request_applied(
                pool_handle,
                get_revoc_reg_request,
                lambda response: response["result"]["data"] is not None,
            )

            # Parse the response to get the revocation registry and its timestamp, then store it in 'revoc_reg_registry' dictionary
            (rev_reg_id, rev_reg_json, timestamp2) = (
                await ledger.parse_get_revoc_reg_response(get_revoc_reg_response)
            )

            revoc_reg_registry[rev_reg_id] = {timestamp2: json.loads(rev_reg_json)}
            revoc_reg_def[rev_reg_id] = json.loads(revoc_reg_def_json)

    # Return the retrieved entities as JSON strings
    return (
        json.dumps(retrieved_schemas),
        json.dumps(retrieved_cred_defs),
        json.dumps(revoc_reg_def),
        json.dumps(revoc_reg_registry),
    )


async def prover_get_entities_from_ledger(
    pool_handle, _did, identifiers, actor, timestamp_from=None, timestamp_to=None
):
    """
    Retrieve schemas, credential definitions, and revocation states from the ledger for the provided identifiers.

    Parameters:
    - pool_handle: Handle to the ledger connection.
    - _did: Decentralized Identifier used for accessing the ledger.
    - identifiers: Dictionary containing schema_id, cred_def_id, rev_reg_id, and optionally rev_reg_seq_no for each entity to retrieve.
    - actor: Identifier for the entity performing the retrieval, used in log messages.
    - timestamp_from (optional): Start time for fetching revocation registry entries.
    - timestamp_to (optional): End time for fetching revocation registry entries.

    Returns:
    - Tuple containing JSON strings of the following:
        - schemas: Dictionary of schema data retrieved from the ledger.
        - cred_defs: Dictionary of credential definitions retrieved from the ledger.
        - rev_states: Dictionary of revocation states retrieved from the ledger.
    """

    # Initialize dictionaries to store retrieved entities
    schemas = {}
    cred_defs = {}
    rev_states = {}

    # Iterate over each item in the 'identifiers' dictionary
    for item in identifiers.values():
        # Print a message indicating that we are getting a schema from the ledger
        print('" {} " -> Get Schema from Ledger'.format(actor))

        # Retrieve a schema from the ledger and store it in the 'schemas' dictionary
        (received_schema_id, received_schema) = await get_schema(
            pool_handle, _did, item["schema_id"]
        )
        schemas[received_schema_id] = json.loads(received_schema)

        # Print a message indicating that we are getting a credential definition from the ledger
        print('" {} " -> Get Credential Definition from Ledger'.format(actor))

        # Retrieve a credential definition from the ledger and store it in the 'cred_defs' dictionary
        (received_cred_def_id, received_cred_def) = await get_cred_def(
            pool_handle, _did, item["cred_def_id"]
        )
        cred_defs[received_cred_def_id] = json.loads(received_cred_def)

        # Check if the item includes information about revocation
        if "rev_reg_seq_no" in item and item["rev_reg_id"] is not None:
            # Print a message indicating that we are getting a revocation registry definition from the ledger
            print(
                '" {} " -> Get Revocation Registry Definition from Ledger'.format(actor)
            )

            # Build a request to get the revocation registry definition
            get_revoc_reg_def_request = await ledger.build_get_revoc_reg_def_request(
                _did, item["rev_reg_id"]
            )

            # Ensure that the previous request is applied and the data is not None
            get_revoc_reg_def_response = await ensure_previous_request_applied(
                pool_handle,
                get_revoc_reg_def_request,
                lambda response: response["result"]["data"] is not None,
            )

            # Parse the response to get the revocation registry definition and store it in 'revoc_reg_def_json'
            (rev_reg_id, revoc_reg_def_json) = (
                await ledger.parse_get_revoc_reg_def_response(
                    get_revoc_reg_def_response
                )
            )

            # Print a message indicating that we are getting a revocation registry delta from the ledger
            print('" {} " -> Get Revocation Registry Delta from Ledger'.format(actor))

            # If 'timestamp_to' is not provided, use the current time
            if not timestamp_to:
                timestamp_to = int(time.time())

            # Build a request to get the revocation registry delta
            get_revoc_reg_delta_request = (
                await ledger.build_get_revoc_reg_delta_request(
                    _did, item["rev_reg_id"], timestamp_from, timestamp_to
                )
            )

            # Ensure that the previous request is applied and the data is not None
            get_revoc_reg_delta_response = await ensure_previous_request_applied(
                pool_handle,
                get_revoc_reg_delta_request,
                lambda response: response["result"]["data"] is not None,
            )

            # Parse the response to get the revocation registry delta and store it in 'revoc_reg_delta_json'
            (rev_reg_id, revoc_reg_delta_json, t) = (
                await ledger.parse_get_revoc_reg_delta_response(
                    get_revoc_reg_delta_response
                )
            )

            # Create configuration for tails reader
            tails_reader_config = json.dumps(
                {
                    "base_dir": dirname(
                        json.loads(revoc_reg_def_json)["value"]["tailsLocation"]
                    ),
                    "uri_pattern": "",
                }
            )

            # Open a reader for blob storage using the tails reader configuration
            blob_storage_reader_cfg_handle = await blob_storage.open_reader(
                "default", tails_reader_config
            )

            # Print a message indicating that we are creating a revocation state
            print('" {} " -> Create Revocation State'.format(actor))

            # Update the revocation state and store it in 'rev_state_json'
            rev_state_json = await anoncreds.verifier_update_revocation_state(
                blob_storage_reader_cfg_handle,
                revoc_reg_def_json,
                revoc_reg_delta_json,
                item["timestamp"],
                item["cred_rev_id"],
            )

            # Store the revocation state in the 'rev_states' dictionary
            rev_states[rev_reg_id] = {t: json.loads(rev_state_json)}

    # Return the retrieved entities as JSON strings
    return json.dumps(schemas), json.dumps(cred_defs), json.dumps(rev_states)


async def create_wallet(identity):
    """
    Create a wallet for the provided identity.

    Parameters:
    - identity: Dictionary containing the identity information.
    """

    # Create a wallet for the identity using the provided configuration and credentials
    try:
        await wallet.create_wallet(
            identity["wallet_config"], identity["wallet_credentials"]
        )
    except IndyError as ex:
        if ex.error_code == ErrorCode.WalletAlreadyExistsError:
            pass

    # Open the wallet and store the returned wallet handle in the identity dictionary
    try:
        identity["wallet"] = await wallet.open_wallet(
            identity["wallet_config"], identity["wallet_credentials"]
        )
    except IndyError as ex:
        if ex.error_code == ErrorCode.WalletAlreadyOpenedError:
            pass


async def getting_verinym(from_, to):
    """
    Create a new DID and cryptographic key pair for the provided identity.
    Register the new DID on the ledger and send a nym request to the ledger.

    Parameters:
    - from_: Dictionary containing the identity information of the entity sending the nym request.
    - to: Dictionary containing the identity information of the entity receiving the nym request.
    """

    # Create a wallet for the entity receiving the nym request
    await create_wallet(to)
    (to["did"], to["key"]) = await did.create_and_store_my_did(to["wallet"], "{}")

    # Prepare the nym request to send to the ledger
    from_["info"] = {
        "did": to["did"],
        "verkey": to["key"],
        "role": to.get("role", None),
    }

    # Send the nym request to the ledger
    await send_nym(
        from_["pool"],
        from_["wallet"],
        from_["did"],
        from_["info"]["did"],
        from_["info"]["verkey"],
        from_["info"]["role"],
    )


async def send_nym(pool_handle, wallet_handle, _did, new_did, new_key, role):
    """
    Send a nym request to the ledger.

    Parameters:
    - pool_handle: Handle to the ledger connection.
    - wallet_handle: Handle to the wallet.
    - _did: Decentralized Identifier used for accessing the ledger.
    - new_did: New Decentralized Identifier to register on the ledger.
    - new_key: Cryptographic key associated with the new Decentralized Identifier.
    - role: Role of the entity being registered on the ledger.
    """

    # Build the nym request to send to the ledger
    nym_request = await ledger.build_nym_request(_did, new_did, new_key, None, role)
    print(f"Nym Request: {nym_request}")
    # Sign and submit the nym request to the ledger
    await ledger.sign_and_submit_request(pool_handle, wallet_handle, _did, nym_request)


async def ensure_previous_request_applied(pool_handle, checker_request, checker):
    """
    Ensure that the previous request is applied successfully.

    Parameters:
    - pool_handle: Handle to the ledger connection.
    - checker_request: Request to check the status of the previous request.
    - checker: Function to check the status of the previous request.

    Returns:
    - JSON string containing the response from the ledger.
    """

    # Check the status of the previous request
    for _ in range(3):
        # Submit the request to the ledger and store the response
        response = json.loads(await ledger.submit_request(pool_handle, checker_request))
        try:
            if checker(response):
                return json.dumps(
                    response
                )  # Return the response if the checker function returns True
        except TypeError:
            pass
        time.sleep(5)  # Wait for 5 seconds before trying again


async def get_cred_def(pool_handle, _did, cred_def_id):
    """
    Retrieve a credential definition from the ledger.

    Parameters:
    - pool_handle: Handle to the ledger connection.
    - _did: Decentralized Identifier used for accessing the ledger.
    - cred_def_id: Credential definition identifier to retrieve.

    Returns:
    - Tuple containing the credential definition identifier and the credential definition.
    """

    # Build a request to get the credential definition
    get_cred_def_request = await ledger.build_get_cred_def_request(_did, cred_def_id)

    # Ensure that the previous request is applied and that the data is not None
    get_cred_def_response = await ensure_previous_request_applied(
        pool_handle,
        get_cred_def_request,
        lambda response: response["result"]["data"] is not None,
    )

    # Parse the response to obtain the credential definition and return it
    return await ledger.parse_get_cred_def_response(get_cred_def_response)


async def get_credential_for_referent(search_handle, referent):
    """
    Retrieve a credential for the given referent from the search handle.

    Parameters:
    - search_handle: Handle to the search for credentials.
    - referent: Referent for which to retrieve the credential.

    Returns:
    - Credential information for the given referent.
    """

    # Fetch credentials for the given referent from the search handle, limiting to 10 credentials
    credentials = json.loads(
        await anoncreds.prover_fetch_credentials_for_proof_req(
            search_handle, referent, 10
        )
    )

    # Extract and return the first credential information from the fetched credentials
    return credentials[0]["cred_info"]


async def get_schema(pool_handle, _did, schema_id):
    """
    Retrieve a schema from the ledger.

    Parameters:
    - pool_handle: Handle to the ledger connection.
    - _did: Decentralized Identifier used for accessing the ledger.
    - schema_id: Schema identifier to retrieve.

    Returns:
    - Tuple containing the schema identifier and the schema.
    """

    # Build a request to get the schema using the provided DID and schema ID
    get_schema_request = await ledger.build_get_schema_request(_did, schema_id)

    # Ensure that the previous request is applied successfully and that the response contains data
    get_schema_response = await ensure_previous_request_applied(
        pool_handle,
        get_schema_request,
        lambda response: response["result"]["data"] is not None,
    )

    # Parse the response to obtain the schema and return it
    return await ledger.parse_get_schema_response(get_schema_response)


async def create_schema(creator, schema_name, schema_version, attributes):
    """
    Create a schema and send it to the ledger.

    Parameters:
    - creator: Dictionary containing the identity information of the entity creating the schema.
    - schema_name: Name of the schema to create.
    - schema_version: Version of the schema to create.
    - attributes: List of attributes for the schema.

    Returns:
    - Tuple containing the schema identifier and the schema.
    """

    # Create a schema and store the schema identifier and schema JSON
    schema_id, schema_json = await anoncreds.issuer_create_schema(
        creator["did"], schema_name, schema_version, json.dumps(attributes)
    )

    # Build a schema request and send it to the ledger
    schema_request = await ledger.build_schema_request(creator["did"], schema_json)

    # Sign and submit the schema request to the ledger
    await ledger.sign_and_submit_request(
        creator["pool"],
        creator["wallet"],
        creator["did"],
        schema_request,
    )

    # Return the schema identifier and schema JSON
    return schema_id, schema_json


async def create_and_store_credential_def(
    wallet, did, schema, tag, cred_def_type, config
):
    """
    Create and store a credential definition in the wallet.

    Parameters:
    - wallet: Handle to the wallet.
    - did: Decentralized Identifier used for accessing the ledger.
    - schema: Schema to create the credential definition.
    - tag: Tag for the credential definition.
    - cred_def_type: Type of the credential definition.
    - config: Configuration for the credential definition.

    Returns:
    - Tuple containing the credential definition identifier and the credential definition.
    """

    # Create and store the credential definition in the wallet
    cred_def_id, cred_def_json = await anoncreds.issuer_create_and_store_credential_def(
        wallet, did, schema, tag, cred_def_type, json.dumps(config)
    )
    return cred_def_id, cred_def_json


async def send_credential_def_to_ledger(did, cred_def, pool, wallet):
    """
    Send a credential definition to the ledger.

    Parameters:
    - did: Decentralized Identifier used for accessing the ledger.
    - cred_def: Credential definition to send to the ledger.
    - pool: Handle to the ledger connection.
    - wallet: Handle to the wallet.
    """

    # Build a credential definition request and send it to the ledger
    cred_def_request = await ledger.build_cred_def_request(did, cred_def)

    # Sign and submit the credential definition request to the ledger
    await ledger.sign_and_submit_request(pool, wallet, did, cred_def_request)


async def run():
    """
    Main function to run the code.
    """

    # Print a message to indicate the start of the program
    print("Theory and Applications of Blockchain: Assignment 5")
    print("\n\n--------------------------------------------")
    # Print the names and roll numbers of the students
    print("Submission by - Bratin Mondal (21CS10016) and Abir Roy (21CS30074)")
    print("\n\n--------------------------------------------")
    # Define a dictionary for pool configuration
    pool_ = {"name": "pool1"}

    # Print a message to indicate Part A
    print("\n\n--------------------------------------------")
    print(f"PART - A")
    print("--------------------------------------------\n\n")

    # Print a message to indicate opening the pool ledger with its name
    print(f"Open Pool Ledger: {pool_['name']}")

    # Set the path to the genesis transaction file for the pool
    pool_["genesis_txn_path"] = "pool1.txn"

    # Create a 'config' entry in the dictionary containing JSON configuration data with the genesis transaction path
    pool_["config"] = json.dumps({"genesis_txn": str(pool_["genesis_txn_path"])})

    # Connecting to the pool and setting the protocol version
    print(f"Setting protocol version 2")
    await pool.set_protocol_version(2)

    # Create the pool ledger configuration
    print(f"Creating pool ledger config: {pool_['name']}")
    try:
        await pool.create_pool_ledger_config(pool_["name"], pool_["config"])
    except IndyError as ex:
        if ex.error_code == ErrorCode.PoolLedgerConfigAlreadyExistsError:
            pass

    # start the docker container if not running
    print("Starting the docker container")
    try:
        command = "docker run -itd -p 9701-9708:9701-9708 mailtisen/indy_pool"
        subprocess.run(
            command, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
    except:
        pass

    # Open the pool ledger and store the returned pool handle in the dictionary
    pool_["handle"] = await pool.open_pool_ledger(pool_["name"], None)
    print("Connected to the pool successfully.")
    print("\n\n--------------------------------------------\n\n")

    # Configure the steward
    print("Configuring Steward...")
    steward = {
        "name": "Mr. Steward",
        "wallet_config": json.dumps({"id": "mr_steward_wallet"}),
        "wallet_credentials": json.dumps({"key": "mr_wallet_key"}),
        "pool": pool_["handle"],
        "seed": "000000000000000000000000Steward1",
    }
    print(f'Creating a wallet for "{steward["name"]}"')

    # Creating a wallet for the steward and generating a DID
    await create_wallet(steward)

    # Define 'did_info' for the steward containing a seed for DID generation
    steward["did_info"] = json.dumps({"seed": steward["seed"]})

    # Print the steward's DID information
    print(steward["did_info"])

    # Generate a DID and cryptographic key pair for the steward and store it in the wallet
    steward["did"], steward["key"] = await did.create_and_store_my_did(
        steward["wallet"], steward["did_info"]
    )

    # Print a success message after configuring the steward
    print("Steward configured successfully.")
    print("\n\n--------------------------------------------\n\n")

    # Register the Research Institute
    print("Registering Research_Institute...")
    research_institute = {
        "name": "Research Institute",
        "wallet_config": json.dumps({"id": "research_institute_wallet"}),
        "wallet_credentials": json.dumps({"key": "research_institute_wallet_key"}),
        "pool": pool_["handle"],
        "role": "TRUST_ANCHOR",
    }
    
    # Create a wallet for the Research Institute
    print(f'Creating a wallet for "{research_institute["name"]}"')

    await getting_verinym(steward, research_institute)
    print("Research_Institute registered successfully.")
    print("\n\n--------------------------------------------\n\n")

    # Register the University
    print("Registering University...")
    university = {
        "name": "University",
        "wallet_config": json.dumps({"id": "university_wallet"}),
        "wallet_credentials": json.dumps({"key": "university_wallet_key"}),
        "pool": pool_["handle"],
        "role": "TRUST_ANCHOR",
    }
    
    # Create a wallet for the University
    print(f'Creating a wallet for "{university["name"]}"')

    await getting_verinym(steward, university)
    print("University registered successfully.")
    print("\n\n--------------------------------------------\n\n")

    print("\n\n--------------------------------------------")
    print(f"PART - B")
    print("--------------------------------------------\n\n")

    print("Research Institute Creates ResearchAuthorship Schema")

    print('"research_institute" -> Create "ResearchAuthorship" Schema')
    # ResearchAuthorship Schema
    ResearchAuthorship_details = {
        "name": "ResearchAuthorship",
        "version": "1.5",
        "attributes": [
            "author_first_name",
            "author_last_name",
            "research_title",
            "institute_name",
            "research_field",
            "publication_year",
        ],
    }
    
    # Create the ResearchAuthorship schema
    research_schema_id, research_schema = await create_schema(
        research_institute,
        ResearchAuthorship_details["name"],
        ResearchAuthorship_details["version"],
        ResearchAuthorship_details["attributes"],
    )
    
    # Store the schema ID and schema JSON in the Research Institute dictionary
    research_institute["ResearchAuthorship_schema_id"] = research_schema_id
    research_institute["ResearchAuthorship_schema"] = research_schema

    # Store the schema ID in a variable for future use
    ResearchAuthorship_id = research_institute["ResearchAuthorship_schema_id"]

    print("ResearchAuthorship Schema created successfully.")

    print("\n\n--------------------------------------------\n\n")

    print("University Creates AcademicQualification Schema")

    print('"university" -> Create "AcademicQualification" Schema')
    # AcademicQualification Schema
    AcademicQualification_student = {
        "name": "AcademicQualification",
        "version": "1.5",
        "attributes": [
            "student_first_name",
            "student_last_name",
            "degree",
            "field_of_study",
            "university_name",
            "graduation_year",
            "cgpa",
        ],
    }

    # Create the AcademicQualification schema
    student_schema_id, student_schema = await create_schema(
        university,
        AcademicQualification_student["name"],
        AcademicQualification_student["version"],
        AcademicQualification_student["attributes"],
    )
    
    # Store the schema ID and schema JSON in the University dictionary
    university["AcademicQualification_student_schema_id"] = student_schema_id
    university["AcademicQualification_student_schema"] = student_schema
    
    # Store the schema ID in a variable for future use
    AcademicQualification_student_id = university[
        "AcademicQualification_student_schema_id"
    ]

    print("AcademicQualification Schema created successfully.")
    print("\n\n--------------------------------------------\n\n")


    # ResearchAuthorship credential definition
    print(
        "Research Instiute Creates credential definition for ResearchAuthorship"
    )
    print('"research_institute" -> Get the schema from the ledger')

    # Request the schema from the ledger for ResearchAuthorship
    get_schema_request = await ledger.build_get_schema_request(
        research_institute["did"], ResearchAuthorship_id
    )
    
    # Ensure that the previous request is applied and the data is not None
    get_schema_response = await ensure_previous_request_applied(
        research_institute["pool"],
        get_schema_request,
        lambda response: response["result"]["data"] is not None,
    )
    (
        research_institute["ResearchAuthorship_schema_id"],
        research_institute["ResearchAuthorship_schema"],
    ) = await ledger.parse_get_schema_response(get_schema_response)

    # transcript credential definition
    print(
        '"research_institute" -> Create and store in the wallet. Credential definition'
    )
    
    # Define the credential definition for ResearchAuthorship
    ResearchAuthorship_cred_def = {
        "tag": "TAG1",
        "type": "CL",
        "config": {"support_revocation": False},
    }

    # Create and store the credential definition for ResearchAuthorship
    (
        research_institute["ResearchAuthorship_cred_def_id"],
        research_institute["ResearchAuthorship_cred_def"],
    ) = await create_and_store_credential_def(
        research_institute["wallet"],
        research_institute["did"],
        research_institute["ResearchAuthorship_schema"],
        ResearchAuthorship_cred_def["tag"],
        ResearchAuthorship_cred_def["type"],
        ResearchAuthorship_cred_def["config"],
    )

    # Send the credential definition to the ledger
    await send_credential_def_to_ledger(
        research_institute["did"],
        research_institute["ResearchAuthorship_cred_def"],
        research_institute["pool"],
        research_institute["wallet"],
    )
    print(
        "research_institute credential definition for ResearchAuthorship registered successfully."
    )

    print("\n\n--------------------------------------------\n\n")

    print("University Creates credential definition for AcademicQualification")
    print('"university" -> Get the schema from the ledger')

    # Request the schema from the ledger for AcademicQualification
    get_schema_request = await ledger.build_get_schema_request(
        university["did"], AcademicQualification_student_id
    )
    
    # Ensure that the previous request is applied and the data is not None
    get_schema_response = await ensure_previous_request_applied(
        university["pool"],
        get_schema_request,
        lambda response: response["result"]["data"] is not None,
    )
    (
        university["AcademicQualification_schema_id"],
        university["AcademicQualification_schema"],
    ) = await ledger.parse_get_schema_response(get_schema_response)

    # Create and store the credential definition for AcademicQualification
    print('"university" -> Create and store in the wallet. Credential definition')
    AcademicQualification_cred_def = {
        "tag": "TAG1",
        "type": "CL",
        "config": {"support_revocation": False},
    }
    # Create and store the credential definition for AcademicQualification
    (
        university["AcademicQualification_cred_def_id"],
        university["AcademicQualification_cred_def"],
    ) = await create_and_store_credential_def(
        university["wallet"],
        university["did"],
        university["AcademicQualification_schema"],
        AcademicQualification_cred_def["tag"],
        AcademicQualification_cred_def["type"],
        AcademicQualification_cred_def["config"],
    )

    # Send the credential definition to the ledger
    await send_credential_def_to_ledger(
        university["did"],
        university["AcademicQualification_cred_def"],
        university["pool"],
        university["wallet"],
    )
    print(
        "university credential definition for AcademicQualification registered successfully."
    )


    print("\n\n--------------------------------------------")
    
    print("PART - C")
    print("--------------------------------------------\n\n")
    # Setting up Arjun
    _Arjun = {
        "name": "Arjun",
        "wallet_config": json.dumps({"id": "Arjun_wallet"}),
        "wallet_credentials": json.dumps({"key": "Arjun_wallet_key"}),
        "pool": pool_["handle"],
    }

    # Create a wallet for Arjun
    await create_wallet(_Arjun)
    (_Arjun["did"], _Arjun["key"]) = await did.create_and_store_my_did(
        _Arjun["wallet"], "{}"
    )

    print("\n\n--------------------------------------------\n\n")
    # Arjun creates and stores a Master Secret in his wallet
    print("Arjun creates and stores a Master Secret in Wallet")
    _Arjun["master_secret_id"] = await anoncreds.prover_create_master_secret(
        _Arjun["wallet"], None
    )
    print("\n\n--------------------------------------------\n\n")

    # Research Institute issues ResearchAuthorship credential to Arjun
    research_institute["ResearchAuthorship_cred_offer"] = (
        await anoncreds.issuer_create_credential_offer(
            research_institute["wallet"],
            research_institute["ResearchAuthorship_cred_def_id"],
        )
    )

    # Send the ResearchAuthorship credential offer to Arjun
    print("Research Institute issues ResearchAuthorship credential to Arjun")
    _Arjun["ResearchAuthorship_cred_offer"] = research_institute[
        "ResearchAuthorship_cred_offer"
    ]

    # Get the ResearchAuthorship credential definition for Arjun from the ledger
    ResearchAuthorship_cred_offer_object = json.loads(
        _Arjun["ResearchAuthorship_cred_offer"]
    )
    _Arjun["ResearchAuthorship_schema_id"] = ResearchAuthorship_cred_offer_object[
        "schema_id"
    ]
    _Arjun["ResearchAuthorship_cred_def_id"] = ResearchAuthorship_cred_offer_object[
        "cred_def_id"
    ]

    (
        _Arjun["research_institute_ResearchAuthorship_cred_def_id"],
        _Arjun["research_institute_ResearchAuthorship_cred_def"],
    ) = await get_cred_def(
        _Arjun["pool"], _Arjun["did"], _Arjun["ResearchAuthorship_cred_def_id"]
    )

    (
        _Arjun["ResearchAuthorship_cred_request"],
        _Arjun["ResearchAuthorship_cred_request_metadata"],
    ) = await anoncreds.prover_create_credential_req(
        _Arjun["wallet"],
        _Arjun["did"],
        _Arjun["ResearchAuthorship_cred_offer"],
        _Arjun["research_institute_ResearchAuthorship_cred_def"],
        _Arjun["master_secret_id"],
    )

    # Store the ResearchAuthorship credential request in Arjun's dictionary
    research_institute["ResearchAuthorship_cred_request"] = _Arjun[
        "ResearchAuthorship_cred_request"
    ]

    # Define the values for the ResearchAuthorship credential
    research_institute["Arjun_ResearchAuthorship_cred_values"] = json.dumps(
        {
            "author_first_name": {"raw": "Arjun", "encoded": "11345123456289765345"},
            "author_last_name": {"raw": "Verma", "encoded": "54217896451236587412"},
            "research_title": {
                "raw": "Innovative Techniques in Sustainable Farming",
                "encoded": "85412635478965321478",
            },
            "institute_name": {
                "raw": "GreenEarth Research Institute",
                "encoded": "65412387965412357896",
            },
            "research_field": {
                "raw": "Environmental Studies",
                "encoded": "98754123657894123657",
            },
            "publication_year": {"raw": "2023", "encoded": "2023"},
        }
    )

    # Create the ResearchAuthorship credential
    research_institute["ResearchAuthorship_cred"], _, _ = (
        await anoncreds.issuer_create_credential(
            research_institute["wallet"],
            research_institute["ResearchAuthorship_cred_offer"],
            research_institute["ResearchAuthorship_cred_request"],
            research_institute["Arjun_ResearchAuthorship_cred_values"],
            None,
            None,
        )
    )

    # Send the ResearchAuthorship credential to Arjun
    print("research_institute sends the ResearchAuthorship credential to Arjun")
    _Arjun["ResearchAuthorship_cred"] = research_institute["ResearchAuthorship_cred"]

    # Get the ResearchAuthorship credential definition for Arjun from the ledger
    _, _Arjun["ResearchAuthorship_cred_def"] = await get_cred_def(
        _Arjun["pool"], _Arjun["did"], _Arjun["ResearchAuthorship_cred_def_id"]
    )

    # Store the ResearchAuthorship credential in Arjun's wallet
    await anoncreds.prover_store_credential(
        _Arjun["wallet"],
        None,
        _Arjun["ResearchAuthorship_cred_request_metadata"],
        _Arjun["ResearchAuthorship_cred"],
        _Arjun["ResearchAuthorship_cred_def"],
        None,
    )
    
    print("Arjun's ResearchAuthorship credential stored in Wallet")

    print("\n\n--------------------------------------------\n\n")

    print("University issues AcademicQualifications credential to Arjun")
    
    # University issues AcademicQualifications credential to Arjun
    university["AcademicQualifications_cred_offer"] = (
        await anoncreds.issuer_create_credential_offer(
            university["wallet"], university["AcademicQualification_cred_def_id"]
        )
    )

    # Send the AcademicQualifications credential offer to Arjun
    _Arjun["AcademicQualifications_cred_offer"] = university[
        "AcademicQualifications_cred_offer"
    ]

    # Get the AcademicQualifications credential definition for Arjun from the ledger
    AcademicQualifications_cred_offer_object = json.loads(
        _Arjun["AcademicQualifications_cred_offer"]
    )
    _Arjun["AcademicQualifications_schema_id"] = (
        AcademicQualifications_cred_offer_object["schema_id"]
    )
    _Arjun["AcademicQualifications_cred_def_id"] = (
        AcademicQualifications_cred_offer_object["cred_def_id"]
    )

    (
        _Arjun["university_AcademicQualifications_cred_def_id"],
        _Arjun["university_AcademicQualifications_cred_def"],
    ) = await get_cred_def(
        _Arjun["pool"], _Arjun["did"], _Arjun["AcademicQualifications_cred_def_id"]
    )

    (
        _Arjun["AcademicQualifications_cred_request"],
        _Arjun["AcademicQualifications_cred_request_metadata"],
    ) = await anoncreds.prover_create_credential_req(
        _Arjun["wallet"],
        _Arjun["did"],
        _Arjun["AcademicQualifications_cred_offer"],
        _Arjun["university_AcademicQualifications_cred_def"],
        _Arjun["master_secret_id"],
    )

    # Store the AcademicQualifications credential request in Arjun's dictionary
    university["AcademicQualifications_cred_request"] = _Arjun[
        "AcademicQualifications_cred_request"
    ]

    # Define the values for the AcademicQualifications credential
    university["_Arjun_AcademicQualifications_cred_values"] = json.dumps(
        {
            "student_first_name": {"raw": "Arjun", "encoded": "11345123456289765345"},
            "student_last_name": {"raw": "Verma", "encoded": "54217896451236587412"},
            "degree": {
                "raw": "PhD in Environmental Science",
                "encoded": "21345789412365789456",
            },
            "field_of_study": {
                "raw": "Sustainable Agriculture",
                "encoded": "89456123478512365489",
            },
            "university_name": {
                "raw": "Delhi University",
                "encoded": "65894123657894123578",
            },
            "graduation_year": {"raw": "2022", "encoded": "2022"},
            "cgpa": {"raw": "9", "encoded": "9"},
        }
    )

    # Create the AcademicQualifications credential
    university["AcademicQualifications_cred"], _, _ = (
        await anoncreds.issuer_create_credential(
            university["wallet"],
            university["AcademicQualifications_cred_offer"],
            university["AcademicQualifications_cred_request"],
            university["_Arjun_AcademicQualifications_cred_values"],
            None,
            None,
        )
    )

    # Send the AcademicQualifications credential to Arjun
    print("university sends the AcademicQualifications credential to Arjun")
    _Arjun["AcademicQualifications_cred"] = university["AcademicQualifications_cred"]

    _, _Arjun["AcademicQualifications_cred_def"] = await get_cred_def(
        _Arjun["pool"], _Arjun["did"], _Arjun["AcademicQualifications_cred_def_id"]
    )
    
    # Store the AcademicQualifications credential in Arjun's wallet
    await anoncreds.prover_store_credential(
        _Arjun["wallet"],
        None,
        _Arjun["AcademicQualifications_cred_request_metadata"],
        _Arjun["AcademicQualifications_cred"],
        _Arjun["AcademicQualifications_cred_def"],
        None,
    )

    print("Arjun's AcademicQualifications credential stored in Wallet")

    print(
        "\n\n--------------------------------------------\n\n",
        _Arjun["ResearchAuthorship_cred_def"],
    )
    print(
        "\n\n--------------------------------------------\n\n",
        _Arjun["AcademicQualifications_cred_def"],
    )
 
    print("\n\n--------------------------------------------\n\n")
    
    
    print("\n\n--------------------------------------------")
    print("PART - D")
    print("--------------------------------------------\n\n")
    
    # IES Organizers
    IES_Organizers = {
        "name": "IES Organizers",
        "wallet_config": json.dumps({"id": "IES_Organizers_wallet"}),
        "wallet_credentials": json.dumps({"key": "IES_Organizers_wallet_key"}),
        "pool": pool_["handle"],
    }
    
    # Create a wallet for IES Organizers
    await create_wallet(IES_Organizers)
    # Generate a DID and cryptographic key pair for IES Organizers
    await getting_verinym(steward, IES_Organizers)
    nonce = await anoncreds.generate_nonce()

    # Bank creates a proof request for presentation eligibility
    IES_Organizers["presentation_eligibility_proof_request"] = json.dumps(
        {
            "nonce": nonce,
            "name": "Eligibility",
            "version": "0.1",
            "requested_attributes": {
                "attr1_referent": {"name": "first_name"},
                "attr2_referent": {"name": "last_name"},
                "attr3_referent": {
                    "name": "degree",
                    "restrictions": [
                        {"cred_def_id": university["AcademicQualification_cred_def_id"]}
                    ],
                },
                "attr4_referent": {
                    "name": "field_of_study",
                    "restrictions": [
                        {"cred_def_id": university["AcademicQualification_cred_def_id"]}
                    ],
                },
                "attr5_referent": {
                    "name": "university_name",
                    "restrictions": [
                        {"cred_def_id": university["AcademicQualification_cred_def_id"]}
                    ],
                },
                "attr6_referent": {
                    "name": "research_title",
                    "restrictions": [
                        {
                            "cred_def_id": research_institute[
                                "ResearchAuthorship_cred_def_id"
                            ]
                        }
                    ],
                },
                "attr7_referent": {
                    "name": "research_field",
                    "restrictions": [
                        {
                            "cred_def_id": research_institute[
                                "ResearchAuthorship_cred_def_id"
                            ]
                        }
                    ],
                },
            },
            "requested_predicates": {
                "predicate1_referent": {
                    "name": "graduation_year",
                    "p_type": ">=",
                    "p_value": 2020,
                    "restrictions": [
                        {"cred_def_id": university["AcademicQualification_cred_def_id"]}
                    ],
                },
                "predicate2_referent": {
                    "name": "graduation_year",
                    "p_type": "<=",
                    "p_value": 2023,
                    "restrictions": [
                        {"cred_def_id": university["AcademicQualification_cred_def_id"]}
                    ],
                },
                "predicate3_referent": {
                    "name": "cgpa",
                    "p_type": ">",
                    "p_value": 6,
                    "restrictions": [
                        {"cred_def_id": university["AcademicQualification_cred_def_id"]}
                    ],
                },
                "predicate4_referent": {
                    "name": "publication_year",
                    "p_type": ">=",
                    "p_value": 2022,
                    "restrictions": [
                        {
                            "cred_def_id": research_institute[
                                "ResearchAuthorship_cred_def_id"
                            ]
                        }
                    ],
                },
            },
        }
    )

    # Arjun gets the proof request
    print("IES Organizers sends proof request to Arjun")
    _Arjun["presentation_eligibility_proof_request"] = IES_Organizers[
        "presentation_eligibility_proof_request"
    ]

    # Arjun gets credentials for the proof request
    print("Arjun gets credentials for the proof request")
    search_for_presentation_eligibility_proof_request = (
        await anoncreds.prover_search_credentials_for_proof_req(
            _Arjun["wallet"], _Arjun["presentation_eligibility_proof_request"], None
        )
    )

    # Arjun gets the credentials for the attributes in the proof request
    print(search_for_presentation_eligibility_proof_request)
    cred_for_attr3 = await get_credential_for_referent(
        search_for_presentation_eligibility_proof_request, "attr3_referent"
    )
    cred_for_attr4 = await get_credential_for_referent(
        search_for_presentation_eligibility_proof_request, "attr4_referent"
    )
    cred_for_attr5 = await get_credential_for_referent(
        search_for_presentation_eligibility_proof_request, "attr5_referent"
    )
    cred_for_attr6 = await get_credential_for_referent(
        search_for_presentation_eligibility_proof_request, "attr6_referent"
    )
    cred_for_attr7 = await get_credential_for_referent(
        search_for_presentation_eligibility_proof_request, "attr7_referent"
    )
    try:
        cred_for_predicate1 = await get_credential_for_referent(
            search_for_presentation_eligibility_proof_request, "predicate1_referent"
        )
        cred_for_predicate2 = await get_credential_for_referent(
            search_for_presentation_eligibility_proof_request, "predicate2_referent"
        )
        cred_for_predicate3 = await get_credential_for_referent(
            search_for_presentation_eligibility_proof_request, "predicate3_referent"
        )
        cred_for_predicate4 = await get_credential_for_referent(
            search_for_presentation_eligibility_proof_request, "predicate4_referent"
        )
    except:
        print("Geneartion of proof request failed !!\nConditions not satisfied") # If the conditions are not satisfied, the proof request will not be generated
        exit(0) # Exit the program

    # Arjun closes the credentials search
    await anoncreds.prover_close_credentials_search_for_proof_req(
        search_for_presentation_eligibility_proof_request
    )

    # Arjun creates the proof for the proof request
    _Arjun["creds_for_presentation_eligibility_proof"] = {
        cred_for_attr3["referent"]: cred_for_attr3,
        cred_for_attr4["referent"]: cred_for_attr4,
        cred_for_attr5["referent"]: cred_for_attr5,
        cred_for_attr6["referent"]: cred_for_attr5,
        cred_for_attr7["referent"]: cred_for_attr5,
        cred_for_predicate1["referent"]: cred_for_predicate1,
        cred_for_predicate2["referent"]: cred_for_predicate2,
        cred_for_predicate3["referent"]: cred_for_predicate3,
        cred_for_predicate4["referent"]: cred_for_predicate4,
    }

    print(_Arjun["creds_for_presentation_eligibility_proof"])

    # Arjun gets the schemas, credential definitions and revocation registries for the proof request
    (
        _Arjun["schemas_for_presentation_eligibility"],
        _Arjun["cred_defs_for_presentation_eligibility"],
        _Arjun["revoc_states_for_presentation_eligibility"],
    ) = await prover_get_entities_from_ledger(
        _Arjun["pool"],
        _Arjun["did"],
        _Arjun["creds_for_presentation_eligibility_proof"],
        _Arjun["name"],
    )

    print('"Arjun creates presentation_eligibility proof')

    # Arjun creates the proof
    _Arjun["presentation_eligibility_requested_creds"] = json.dumps(
        {
            "self_attested_attributes": {
                "attr1_referent": "Arjun",
                "attr2_referent": "Verma",
            },
            "requested_attributes": {
                "attr3_referent": {
                    "cred_id": cred_for_attr3["referent"],
                    "revealed": True,
                },
                "attr4_referent": {
                    "cred_id": cred_for_attr4["referent"],
                    "revealed": True,
                },
                "attr5_referent": {
                    "cred_id": cred_for_attr5["referent"],
                    "revealed": True,
                },
                "attr6_referent": {
                    "cred_id": cred_for_attr6["referent"],
                    "revealed": True,
                },
                "attr7_referent": {
                    "cred_id": cred_for_attr7["referent"],
                    "revealed": True,
                },
            },
            "requested_predicates": {
                "predicate1_referent": {"cred_id": cred_for_predicate1["referent"]},
                "predicate2_referent": {"cred_id": cred_for_predicate2["referent"]},
                "predicate3_referent": {"cred_id": cred_for_predicate3["referent"]},
                "predicate4_referent": {"cred_id": cred_for_predicate4["referent"]},
            },
        }
    )

    # Arjun creates the proof
    try:
        _Arjun["presentation_eligibility_proof"] = await anoncreds.prover_create_proof(
            _Arjun["wallet"],
            _Arjun["presentation_eligibility_proof_request"],
            _Arjun["presentation_eligibility_requested_creds"],
            _Arjun["master_secret_id"],
            _Arjun["schemas_for_presentation_eligibility"],
            _Arjun["cred_defs_for_presentation_eligibility"],
            _Arjun["revoc_states_for_presentation_eligibility"],
        )
    except IndyError as e:
        print(f"An error occurred: {e.error_code} - {e.message}")

    print(_Arjun["presentation_eligibility_proof"])
    print('"Arjun" send proof to IES organizer')

    # IES Organizers validate the proof
    IES_Organizers["presentation_eligibility_proof"] = _Arjun[
        "presentation_eligibility_proof"
    ]

    print("\n\n--------------------------------------------\n\n")

    print("Bank validates Arjun Claims")

    job_application_proof_object = json.loads(
        IES_Organizers["presentation_eligibility_proof"]
    )

    (
        IES_Organizers["schemas_for_presentation_eligibility"],
        IES_Organizers["cred_defs_for_presentation_eligibility"],
        IES_Organizers["revoc_ref_defs_for_presentation_eligibility"],
        IES_Organizers["revoc_regs_for_presentation_eligibility"],
    ) = await verifier_get_entities_from_ledger(
        IES_Organizers["pool"],
        IES_Organizers["did"],
        job_application_proof_object["identifiers"],
        IES_Organizers["name"],
    )

    print("\n\n--------------------------------------------\n\n")
    
    print('IES Organizers" -> Verify "Presentation Eligibility proof from Arjun')

    # Verify the proof provided by Arjun
    try:
        assert (
            "Arjun"
            == job_application_proof_object["requested_proof"]["self_attested_attrs"][
                "attr1_referent"
            ]
        )
        assert (
            "Verma"
            == job_application_proof_object["requested_proof"]["self_attested_attrs"][
                "attr2_referent"
            ]
        )
        assert (
            "PhD in Environmental Science"
            == job_application_proof_object["requested_proof"]["revealed_attrs"][
                "attr3_referent"
            ]["raw"]
        )
        assert (
            "Sustainable Agriculture"
            == job_application_proof_object["requested_proof"]["revealed_attrs"][
                "attr4_referent"
            ]["raw"]
        )
        assert (
            "Delhi University"
            == job_application_proof_object["requested_proof"]["revealed_attrs"][
                "attr5_referent"
            ]["raw"]
        )
        assert (
            "Innovative Techniques in Sustainable Farming"
            == job_application_proof_object["requested_proof"]["revealed_attrs"][
                "attr6_referent"
            ]["raw"]
        )
        assert (
            "Environmental Studies"
            == job_application_proof_object["requested_proof"]["revealed_attrs"][
                "attr7_referent"
            ]["raw"]
        )

        assert await anoncreds.verifier_verify_proof(
            IES_Organizers["presentation_eligibility_proof_request"],
            IES_Organizers["presentation_eligibility_proof"],
            IES_Organizers["schemas_for_presentation_eligibility"],
            IES_Organizers["cred_defs_for_presentation_eligibility"],
            IES_Organizers["revoc_ref_defs_for_presentation_eligibility"],
            IES_Organizers["revoc_regs_for_presentation_eligibility"],
        )
        print("Verification of proof successful!!")
    except AssertionError as e:
        print("Proof Verification Failed!!")


# Run the asyncio event loop to execute the program
loop = asyncio.get_event_loop()
loop.run_until_complete(run())
