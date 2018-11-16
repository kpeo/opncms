{
	"opncms" : {
		"script" : "/opncms",
		"media" :"/templates",
		"syntax_highlighter" : "/templates/sh",
		"disable_registration" : false,
		"languages" : {
			"en" : "en_US.UTF-8" ,
			"ru" : "ru_RU.UTF-8"
		},
                // Sqlite3 Sample Connection String
                "connection_string" : "sqlite3:db=./db/opncms.db;@pool_size=16",
                //
                // PostgreSQL Sample Connection String
                // "connection_string" : "postgresql:dbname=wikipp;@pool_size=16",
                //
                // MySQL Sample Connection String
                //
                // "connection_string" : "mysql:database=wikipp;user=root;password=root;@pool_size=16",
                //
                // In Some cases mysql works faster without prepared statements as it uses query cache, so you
                // may change this string to:
                //
                // "connection_string" : "mysql:database=wikipp;user=root;password=root;@pool_size=16;@use_prepared=off",
                //      
		"templates" : {
			"default" : "/var/www/opncms/scm/default.tmp",
			"users" : "/var/www/opncms/scm/users.tmp",
		},
		"sql" : {
                        "driver" : "sqlite3",
			"params" : {
				"dbname" : "opncms.db",
				"sqlite3_dbdir" : "./db/"
			}
		},
                "file" : {
                        "root" : "/tmp",
			"params" : {
				"path" : "/tmp"
			}
                },

                // Sqlite3 Sample Connection String
//                "connection_string" : "sqlite3:db=./db/wikipp.db;@pool_size=16",
                //
                // PostgreSQL Sample Connection String
                // "connection_string" : "postgresql:dbname=wikipp;@pool_size=16",
                //
                // MySQL Sample Connection String
                //
                // "connection_string" : "mysql:database=wikipp;user=root;password=root;@pool_size=16",
                //
                // In Some cases mysql works faster without prepared statements as it uses query cache, so you
                // may change this string to:
                //
                // "connection_string" : "mysql:database=wikipp;user=root;password=root;@pool_size=16;@use_prepared=off",
                //      


		"auth" : "sql",
		"store" : "sql"
	},
	"service" : {
		//"worker_processes" : 1,
		"worker_threads" : 5,
		//"api" : "http",
		"api" : "fastcgi",
		//"ip" : "192.168.1.102",
		//"port" : 8091
		"socket" : "/tmp/opncms.sock"
		//"socket" : "stdin"
	},
	"session" : {
		"files_dir" : "session",
		"expire" : "renew",
		"location" : "client",
		"timeout" : 100000,
		"cookies" :  {
			"prefix" : "opncms"
		},
		"server" : {
			"storage" : "files"
		},
		"client" : {
			"encryptor" : "aes",
			"key" : "9bc6dbda707cb72ea1205dd5b1c90464"
		}
	},
	"views" : {
		"paths" : [ "./"] ,
		"skins" : [ "opncms_view" , "admin_view" , "user_view" ] ,
		"auto_reload" : true
	},
	"file_server" : {
		"enable": true,
		"doument_root" : "."
	},
	"localization" : {
//		"encoding" : "UTF-8",
		"messages" : {
			"paths" : [ "./build/locale"],
			"domains" :  [ "opncms" ]
		},
		"locales" : [ "en_US.UTF-8", "ru_RU.UTF-8" ]
	},
	"http" : {
		"script" : "/opncms"
	},
	"logging" : {
		"level" : "debug",
		"syslog" : {
			"enable": false,
			"id" : "opncms"
		},
		"file" : {
			"name" : "./opncms.log",
			"append" : true
		}
	},
	"cache" : {
		"backend" : "thread_shared",
		"limit" : 100, // items - thread cache
	}
}

