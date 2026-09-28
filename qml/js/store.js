.pragma library

var DB_NAME = "harbour-radiox";
var DB_VERSION = "1.0";
var DB_DESCRIPTION = "radio x cache";
var DB_SIZE = 2000000;

var BASE_URL = "https://www.radiox.de";

function openDb() {
    return LocalStorage.openDatabaseSync(DB_NAME, DB_VERSION, DB_DESCRIPTION, DB_SIZE);
}

function cacheSchedule(html) {
    var db = openDb();
    db.transaction(function(tx) {
        tx.executeSql("CREATE TABLE IF NOT EXISTS cache(key TEXT PRIMARY KEY, value TEXT, updated TEXT)");
        tx.executeSql("INSERT OR REPLACE INTO cache (key, value, updated) VALUES (?, ?, ?)",
                       ["schedule", html, new Date().toISOString()]);
    });
}

function loadCachedSchedule() {
    var db = openDb();
    var result = "";
    db.readTransaction(function(tx) {
        tx.executeSql("CREATE TABLE IF NOT EXISTS cache(key TEXT PRIMARY KEY, value TEXT, updated TEXT)");
        var rs = tx.executeSql("SELECT value FROM cache WHERE key=?", ["schedule"]);
        if (rs.rows.length > 0) result = rs.rows.item(0).value;
    });
    return result;
}

function cacheRecordings(html) {
    var db = openDb();
    db.transaction(function(tx) {
        tx.executeSql("CREATE TABLE IF NOT EXISTS cache(key TEXT PRIMARY KEY, value TEXT, updated TEXT)");
        tx.executeSql("INSERT OR REPLACE INTO cache (key, value, updated) VALUES (?, ?, ?)",
                       ["recordings", html, new Date().toISOString()]);
    });
}

function loadCachedRecordings() {
    var db = openDb();
    var result = "";
    db.readTransaction(function(tx) {
        tx.executeSql("CREATE TABLE IF NOT EXISTS cache(key TEXT PRIMARY KEY, value TEXT, updated TEXT)");
        var rs = tx.executeSql("SELECT value FROM cache WHERE key=?", ["recordings"]);
        if (rs.rows.length > 0) result = rs.rows.item(0).value;
    });
    return result;
}