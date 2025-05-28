#include <iostream>
#include <string>
#include <vector>
#include <oci.h> // Oracle Call Interface header

// Function to handle OCI errors
void checkerr(OCIError *errhp, sword status, bool exit_on_error = true) {
    text errbuf[512];
    sb4 errcode = 0;

    if (status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO) {
        if (status == OCI_SUCCESS_WITH_INFO) {
            std::cout << "OCI Success with info." << std::endl;
        }
        return;
    }

    switch (status) {
        case OCI_ERROR:
            OCIErrorGet((dvoid *)errhp, (ub4)1, (text *)NULL, &errcode, errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);
            std::cerr << "OCI Error: " << (char *)errbuf << " (Code: " << errcode << ")" << std::endl;
            break;
        case OCI_NEED_DATA:
            std::cerr << "OCI Error: OCI_NEED_DATA" << std::endl;
            break;
        case OCI_NO_DATA:
            std::cerr << "OCI Info: OCI_NO_DATA" << std::endl; // Often not an error, but an expected condition
            break;
        case OCI_INVALID_HANDLE:
            std::cerr << "OCI Error: OCI_INVALID_HANDLE" << std::endl;
            break;
        case OCI_STILL_EXECUTING:
            std::cerr << "OCI Error: OCI_STILL_EXECUTING" << std::endl;
            break;
        case OCI_CONTINUE:
            std::cerr << "OCI Error: OCI_CONTINUE" << std::endl;
            break;
        default:
            std::cerr << "OCI Error: Unknown error status " << status << std::endl;
            OCIErrorGet((dvoid *)errhp, (ub4)1, (text *)NULL, &errcode, errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);
            if (errcode != 0) {
                 std::cerr << "    Details: " << (char *)errbuf << std::endl;
            }
            break;
    }

    if (exit_on_error && (status != OCI_NO_DATA && status != OCI_SUCCESS_WITH_INFO)) {
        // Clean up attempts should be made before exiting, but for simplicity here, we exit.
        // In a real app, you'd jump to a cleanup section or use RAII.
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <username> <password> <tns_alias>" << std::endl;
        return 1;
    }

    std::string username = argv[1];
    std::string password = argv[2];
    std::string tns_alias = argv[3];

    OCIEnv *envhp = nullptr;
    OCIError *errhp = nullptr;
    OCISvcCtx *svchp = nullptr;
    OCIServer *srvhp = nullptr;
    OCISession *authp = nullptr;
    OCIStmt *stmthp = nullptr;

    sword status;

    // Initialize OCI environment
    status = OCIEnvCreate((OCIEnv **)&envhp, (ub4)OCI_DEFAULT, (dvoid *)0,
                          (dvoid * (*)(dvoid *, size_t))0,
                          (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                          (void (*)(dvoid *, dvoid *))0, (size_t)0, (dvoid **)0);
    if (status != OCI_SUCCESS) {
        std::cerr << "OCIEnvCreate failed." << std::endl;
        return 1;
    }

    // Allocate error handle
    status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, OCI_HTYPE_ERROR, (size_t)0, (dvoid **)0);
    if (status != OCI_SUCCESS) {
        std::cerr << "OCIHandleAlloc for errhp failed." << std::endl;
        OCIHandleFree(envhp, OCI_HTYPE_ENV);
        return 1;
    }

    // Allocate server handle
    status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&srvhp, OCI_HTYPE_SERVER, (size_t)0, (dvoid **)0);
    checkerr(errhp, status);

    // Allocate service context handle
    status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp, OCI_HTYPE_SVCCTX, (size_t)0, (dvoid **)0);
    checkerr(errhp, status);

    // Attach to server
    status = OCIServerAttach(srvhp, errhp, (text *)tns_alias.c_str(), (sb4)tns_alias.length(), OCI_DEFAULT);
    checkerr(errhp, status);

    // Set server handle in service context
    status = OCIAttrSet((dvoid *)svchp, OCI_HTYPE_SVCCTX, (dvoid *)srvhp, (ub4)0, OCI_ATTR_SERVER, errhp);
    checkerr(errhp, status);

    // Allocate session handle
    status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&authp, OCI_HTYPE_SESSION, (size_t)0, (dvoid **)0);
    checkerr(errhp, status);

    // Set username in session handle
    status = OCIAttrSet((dvoid *)authp, OCI_HTYPE_SESSION, (dvoid *)username.c_str(), (ub4)username.length(), OCI_ATTR_USERNAME, errhp);
    checkerr(errhp, status);

    // Set password in session handle
    status = OCIAttrSet((dvoid *)authp, OCI_HTYPE_SESSION, (dvoid *)password.c_str(), (ub4)password.length(), OCI_ATTR_PASSWORD, errhp);
    checkerr(errhp, status);

    // Begin session
    status = OCISessionBegin(svchp, errhp, authp, OCI_CRED_RDBMS, (ub4)OCI_DEFAULT);
    checkerr(errhp, status);

    // Set session handle in service context
    status = OCIAttrSet((dvoid *)svchp, OCI_HTYPE_SVCCTX, (dvoid *)authp, (ub4)0, OCI_ATTR_SESSION, errhp);
    checkerr(errhp, status);

    std::cout << "Successfully connected to Oracle Database." << std::endl;

    // Allocate statement handle
    status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&stmthp, OCI_HTYPE_STMT, (size_t)0, (dvoid **)0);
    checkerr(errhp, status);

    // Prepare SQL statement
    std::string sql_query = "SELECT USER, TO_CHAR(SYSDATE, 'YYYY-MM-DD HH24:MI:SS') FROM DUAL";
    status = OCIStmtPrepare(stmthp, errhp, (text *)sql_query.c_str(), (ub4)sql_query.length(), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT);
    checkerr(errhp, status);

    // Define output variables
    char user_val[128]; // Buffer for USER
    char sysdate_val[30]; // Buffer for SYSDATE as string "YYYY-MM-DD HH24:MI:SS"

    OCIDefine *defnp1 = nullptr;
    OCIDefine *defnp2 = nullptr;

    status = OCIDefineByPos(stmthp, &defnp1, errhp, 1, (dvoid *)user_val, (sb4)sizeof(user_val), SQLT_STR, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
    checkerr(errhp, status);

    status = OCIDefineByPos(stmthp, &defnp2, errhp, 2, (dvoid *)sysdate_val, (sb4)sizeof(sysdate_val), SQLT_STR, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
    checkerr(errhp, status);

    // Execute statement
    // For a SELECT statement, iters=0 means don't prefetch rows, execute only.
    // iters=1 would execute and fetch first row if using OCIStmtExecuteAndFetch
    status = OCIStmtExecute(svchp, stmthp, errhp, (ub4)0, (ub4)0, (const OCISnapshot *)NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
    if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO && status != OCI_NO_DATA) {
         checkerr(errhp, status); // Will exit if it's a critical error
    }


    std::cout << "Query executed. Fetching results..." << std::endl;

    // Fetch rows
    // OCIStmtFetch2 is recommended over OCIStmtFetch
    status = OCIStmtFetch2(stmthp, errhp, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT);
    if (status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO) {
        std::cout << "User: " << user_val << std::endl;
        std::cout << "SYSDATE: " << sysdate_val << std::endl;
    } else if (status == OCI_NO_DATA) {
        std::cout << "No data found." << std::endl;
    } else {
        checkerr(errhp, status); // Handle other errors
    }

cleanup:
    if (stmthp) {
        status = OCIHandleFree((dvoid *)stmthp, OCI_HTYPE_STMT);
        // Minimal error check for cleanup phase
        if (status != OCI_SUCCESS && errhp) {
            std::cerr << "Error freeing statement handle." << std::endl;
        }
    }
    if (authp && svchp && errhp) { // Only try to end session if session, service context and error handles are valid
        status = OCISessionEnd(svchp, errhp, authp, OCI_DEFAULT);
        if (status != OCI_SUCCESS) {
             // Don't call checkerr here as it might exit. Log and continue cleanup.
            text errbuf_cleanup[512]; sb4 errcode_cleanup = 0;
            OCIErrorGet((dvoid *)errhp, (ub4)1, (text *)NULL, &errcode_cleanup, errbuf_cleanup, (ub4)sizeof(errbuf_cleanup), OCI_HTYPE_ERROR);
            std::cerr << "Error ending session: " << (char*)errbuf_cleanup << " (Code: " << errcode_cleanup << ")" << std::endl;
        }
    }
    if (srvhp && errhp) { // Only detach if server and error handles are valid
        status = OCIServerDetach(srvhp, errhp, OCI_DEFAULT);
         if (status != OCI_SUCCESS) {
            text errbuf_cleanup[512]; sb4 errcode_cleanup = 0;
            OCIErrorGet((dvoid *)errhp, (ub4)1, (text *)NULL, &errcode_cleanup, errbuf_cleanup, (ub4)sizeof(errbuf_cleanup), OCI_HTYPE_ERROR);
            std::cerr << "Error detaching from server: " << (char*)errbuf_cleanup << " (Code: " << errcode_cleanup << ")" << std::endl;
        }
    }
    if (svchp) OCIHandleFree((dvoid *)svchp, OCI_HTYPE_SVCCTX);
    if (srvhp) OCIHandleFree((dvoid *)srvhp, OCI_HTYPE_SERVER);
    if (authp) OCIHandleFree((dvoid *)authp, OCI_HTYPE_SESSION);
    if (errhp) OCIHandleFree((dvoid *)errhp, OCI_HTYPE_ERROR);
    if (envhp) OCIHandleFree((dvoid *)envhp, OCI_HTYPE_ENV);

    std::cout << "Cleaned up OCI handles." << std::endl;

    return 0; // Assuming success if we reached here after query execution
              // A more robust solution would track if any error occurred and return non-zero
}

/*
Error Handling Notes:
The `checkerr` function is a basic error handler. In a real-world application:
1. It should not call `exit()` directly but rather signal the calling code to handle cleanup.
   This could be done by returning a boolean or throwing an exception (if C++ exceptions are used).
2. The `cleanup` section with `goto` is a C-style approach. Modern C++ would prefer RAII
   (Resource Acquisition Is Initialization) where handles are wrapped in classes that
   automatically free them in their destructors. This makes cleanup more robust, especially
   with multiple exit points or exceptions.
3. `OCI_SUCCESS_WITH_INFO` should often be checked for more details, as it might indicate
   issues like data truncation, etc., though for this simple query, it's less critical.
4. `OCI_NO_DATA` is explicitly handled after `OCIStmtFetch2` as it's an expected outcome if the query
   returns no rows (though DUAL always returns one).

SYSDATE Formatting:
The query uses `TO_CHAR(SYSDATE, 'YYYY-MM-DD HH24:MI:SS')` to convert the date to a string.
This simplifies fetching as we can retrieve it directly into a character buffer (SQLT_STR).
Handling Oracle's native DATE type (SQLT_DAT or SQLT_ODT) directly in C/C++ requires more
complex structures (OCIDate) and conversion functions.
*/
