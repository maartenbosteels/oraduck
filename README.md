## C++ Oracle Database Connector

A simple C++ application to connect to an Oracle database using OCI, execute a query, and display results.

## Prerequisites

*   An Oracle Database instance accessible from your machine.
*   Oracle Instant Client (Basic, SQL*Plus, and SDK packages) installed. Version 19c or compatible.
*   `g++` compiler (supporting C++11 or later).
*   `make` utility.
*   The `libaio` library (e.g., `libaio1` or `libaio1t64` on Debian/Ubuntu, `libaio` on RHEL/CentOS).

## Environment Setup

Before compiling and running the application, ensure the following environment variables are correctly set. The exact path for `ORACLE_HOME` will depend on where you unzipped the Instant Client files. The installation step used `/opt/oracle/instantclient_19_27/instantclient_19_27`.

1.  **`ORACLE_HOME`**: Points to your Instant Client directory.
    ```bash
    export ORACLE_HOME=/opt/oracle/instantclient_19_27/instantclient_19_27
    ```

2.  **`LD_LIBRARY_PATH`**: Includes the directory containing OCI libraries.
    ```bash
    export LD_LIBRARY_PATH=$ORACLE_HOME:$LD_LIBRARY_PATH
    # Or, if your Instant Client libraries are in a 'lib' subdirectory:
    # export LD_LIBRARY_PATH=$ORACLE_HOME/lib:$LD_LIBRARY_PATH
    ```
    *Note: The installation script for Instant Client put libraries directly in `$ORACLE_HOME`.*

3.  **`PATH`** (Optional, for `sqlplus` and other utilities):
    ```bash
    export PATH=$ORACLE_HOME:$PATH
    ```

4.  **`TNS_ADMIN`** (Optional, if using TNS aliases):
    If you plan to use TNS names (e.g., `ORCLPDB1`) for the connection string, you'll need a `tnsnames.ora` file.
    Create this file (e.g., in `$ORACLE_HOME/network/admin`) and set `TNS_ADMIN` to point to its directory.
    Example `tnsnames.ora` entry:
    ```
    MYDB =
      (DESCRIPTION =
        (ADDRESS = (PROTOCOL = TCP)(HOST = your-db-host)(PORT = 1521))
        (CONNECT_DATA =
          (SERVER = DEDICATED)
          (SERVICE_NAME = your-service-name)
        )
      )
    ```
    Then set the variable:
    ```bash
    export TNS_ADMIN=$ORACLE_HOME/network/admin
    # Or any other directory where your tnsnames.ora is located.
    # You might need to create the 'network/admin' subdirectories if they don't exist.
    ```
    Alternatively, you can use the full Easy Connect string directly when running the application, e.g., `your-db-host:1521/your-service-name`.

To make these settings persistent, add them to your shell's configuration file (e.g., `~/.bashrc` or `~/.zshrc`) and source it or open a new terminal. The installation step should have added these to `~/.bashrc`.

## Compilation

Navigate to the directory containing `oracle_connect.cpp` and `Makefile`, then run:
```bash
make
```
This will produce an executable named `oracle_connect`.

To clean up build files:
```bash
make clean
```

## Running the Application

Execute the compiled program with your database username, password, and TNS alias (or full connection string) as arguments:

```bash
./oracle_connect <username> <password> <tns_alias_or_connection_string>
```

**Examples:**

*   Using a TNS alias `MYDB` (defined in `tnsnames.ora`):
    ```bash
    ./oracle_connect scott tiger MYDB
    ```

*   Using an Easy Connect string:
    ```bash
    ./oracle_connect hr mypassword localhost:1521/XEPDB1
    ```

The application will output the current database user and system date upon successful connection and query execution.

## Troubleshooting

*   **`error while loading shared libraries: libclntsh.so.19.1: cannot open shared object file: No such file or directory`**:
    This usually means `LD_LIBRARY_PATH` is not set correctly or does not point to the directory containing the Oracle Instant Client libraries. Verify `ORACLE_HOME` and `LD_LIBRARY_PATH`.
*   **OCI Errors**: The application includes basic OCI error reporting. Consult the Oracle documentation for specific OCI error codes if you encounter issues during database operations.
*   **`sqlplus: error while loading shared libraries: libaio.so.1: cannot open shared object file`**: Ensure `libaio1` (or equivalent) is installed. The installation script attempted to handle this by creating a symlink if a versioned libaio was found (e.g. libaio.so.1t64). If problems persist, ensure the symlink `$ORACLE_HOME/libaio.so.1` points to your system's `libaio.so.1`.
