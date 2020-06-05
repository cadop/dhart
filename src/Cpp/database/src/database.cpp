#include "pch.h"
#include <iostream>
#include <sqlite3.h>
#include <string>

namespace HF {
	namespace DB {
		
		/// <summary>
		/// A wrapper for a sqlite database. Automatically closes upon being destroyed. 
		/// </summary>
		class Connection {
			
			/// <summary>
			/// Opens a connection via the string specified by path
			/// </summary>
			/// <param name="path">The name of connection to open</param>
			void OpenConnection(std::string path);
		public:
			sqlite3* conn; ///< Current connection to the database ///<
			
			/// <summary>
			/// Create a new database connection
			/// </summary>
			/// <param name="path"> Path to the database</param>
			Connection(std::string path);

			/// <summary>
			/// Close the existing connection
			/// </summary>
			void Close();

			/// <summary>
			/// Close the existing connection for the database
			/// </summary>
			~Connection();

		};
		/// <summary>
		/// Mantains state for connecting to, storing information in, and reading information from sqlite
		/// </summary>
		class Database {

		private:
			std::string database_path; // Path to the sqlite database

			/// <summary>
			/// Create a new database connection.
			/// </summary>
			Connection openDB();

		public:
			/// <summary>
			/// Create a new db instance from absolute path to a database.
			/// If one doesn't exist already, then one will be created
			/// </summary>
			/// <param name="path"></param>
			Database(std::string path);
		};
	}
}

