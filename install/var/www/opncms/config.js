{
	"opncms" : {
		//specific opnCMS settings
		"brand" : "NewSite",
		"script" : "/opncms",
		"media" : "/static",
		"syntax_highlighter" : "/static/sh",
		"disable_registration" : false,

		"localization" : {
			"locales" : ["en_US.UTF-8","ru_RU.UTF-8"],
	                "messages" : {
        	                "paths" : [ "/var/www/opncms/locale"],
                	        "domains" :  [ "opncms" ]
	                },
//			"en" : "en_US.UTF-8",
//			"ru" : "ru_RU.UTF-8",
		},
		// Sqlite3 Sample Connection String
		//"connection_string" : "sqlite3:db=./db/opncms.db;@pool_size=16",
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
		"data": {
			"driver" : "sqlite3", //sql
			"settings" : {
				"db" : "./db/opncms.db", // WARNING! This is the test database. Be sure the production database is in safe place
				"@pool_size" : "16"
			}
		},
		"file" : {
			"root" : "/tmp",
			"params" : {
				"path" : "/tmp"
			}
		},
		"auth" : {
			"storage" : "data",
			"method" : "bcrypt",
			"rounds" : 15 //12 by default, 20 - for good servers
		},
		"email" : {
			"user" : "",
			"password" : "",
			"tlspassword" : "",
			"direct" : false
		}
		// Sqlite3 Sample Connection String
		//"connection_string" : "sqlite3:db=./db/wikipp.db;@pool_size=16",
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
		"location" : "client", //client,server,both
		"timeout" : 100000,
		"expire" : "renew", //fixed,renew,browser
		"files_dir" : "session",
		"cookies" :  {
			"prefix" : "opncms_project" //please, use unique prefix for each application
			//"domain" : "",
			//"path" : "/",
			//"secure" : false,
			//"expiration_method" : "both", //both,max-age,expires
			//"time_shift" : 0
		},
		"server" : {
			"storage" : "files" //files - dir,shared; memory; network - ips,ports; external - shared_object,settings
		},
		"client" : {
			"hmac" : "sha512",
			"hmac_key" : "8c5223a21e432f5539075c4c560af3534a159705f4699e3314c207bd6c854501ba481c460754427651918f42155750eed236a13dadc65acc1f5ba3bc628bb152",
			//"encryptor" : "aes",
			"key" : "9bc6dbda707cb72ea1205dd5b1c90465"
			//hmac - hmac_key,hmac_key_file
			//cbc - cbc_key,cbc_key_file
			//encryptor - aes, key
		}
	},
//	"forwarding" : {
//		"rules" : [
//			{  
//				"ip" : "192.168.1.10",
//				"port" : 5483,
//				//"host" : "",
//				"script_name" : "/shared/app",
//				"path_info" : "/request/(\\d+)"
//			}
//		]
//	},
//	"misk" : {
//		"invalid_url_throws" : false // /this_is_an_invalid_url_generated_by_url_mapper  
//	},
	"views" : {
		"paths" : [ "./"] ,
		"skins" : [ "opncms_view" , "user_view" , "admin_view" ] ,
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
//		"backend" : "thread_shared",
		"backend" : "process_shared",
		"limit" : 100, // items - thread cache
	}
}
