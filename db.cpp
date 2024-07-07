#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include "config.hpp"
#include "db.hpp"

SQLHANDLE sql_env_handle;
SQLHANDLE sql_connection_handle;
SQLHANDLE sql_statement_handle;
SQLRETURN retcode;
bool initialized = false;

void show_error(unsigned int handletype, const SQLHANDLE& handle)
{
    SQLCHAR sqlstate[1024];
    SQLCHAR message[1024];
    if(SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
        std::cout << "Message: " << message << "\nSQLSTATE: " << sqlstate << std::endl;
}

void sql_cleanup()
{
    if (sql_statement_handle != SQL_NULL_HSTMT)
        SQLFreeHandle(SQL_HANDLE_STMT, sql_statement_handle);
    if (sql_connection_handle != SQL_NULL_HDBC)
    {
        SQLDisconnect(sql_connection_handle);
        SQLFreeHandle(SQL_HANDLE_DBC, sql_connection_handle);
    }
    if (sql_env_handle != SQL_NULL_HENV)
        SQLFreeHandle(SQL_HANDLE_ENV, sql_env_handle);
    initialized = false;
}

SQLRETURN sql_initialize()
{
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_env_handle)
     || SQL_SUCCESS != SQLSetEnvAttr(sql_env_handle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0)
     || SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sql_env_handle, &sql_connection_handle)
     || SQL_SUCCESS != SQLSetConnectAttr(sql_connection_handle, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0))
    {
        sql_cleanup();
        return SQL_ERROR;
    }

    std::cout << "Connecting to the SQL Server, this might take a while..." << std::endl;
    SQLCHAR retconstring[1024];
    retcode = SQLDriverConnect(sql_connection_handle, 
                               NULL, 
                               (SQLCHAR*)MSSQL_CONNECTION_STRING,
                               SQL_NTS, 
                               retconstring, 
                               1024, 
                               NULL,
                               SQL_DRIVER_NOPROMPT);
    // Check for connection success
    if (SQL_FAILED(retcode))
    {
        show_error(SQL_HANDLE_DBC, sql_connection_handle);
        sql_cleanup();
        return retcode;
    }

    std::cout << "Successfully connected to SQL Server" << std::endl;

    // Allocate statement handle
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sql_connection_handle, &sql_statement_handle))
    {
        sql_cleanup();
        return SQL_ERROR;
    }

    // Execute a simple query
    if (SQL_SUCCESS != SQLExecDirect(sql_statement_handle, (SQLCHAR*)"SELECT @@VERSION", SQL_NTS))
    {
        show_error(SQL_HANDLE_STMT, sql_statement_handle);
        sql_cleanup();
        return SQL_ERROR;
    }
    else
    {
        SQLCHAR sql_version[255];
        while (SQLFetch(sql_statement_handle) == SQL_SUCCESS)
        {
            SQLGetData(sql_statement_handle, 1, SQL_C_CHAR, sql_version, 255, NULL);
            std::cout << "SQL Server Version: " << sql_version << std::endl;
        }
    }
    initialized = true;
    return SQL_SUCCESS;
}
