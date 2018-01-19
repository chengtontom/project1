#!/usr/bin/python2.7 
import MySQLdb

class DB_ENTRY:
    'db entry'
    def __init__(self):
        self.time = 0
        self.value = 0.0

class DB_TBL:
    'db table'
    def __init__(self,tbl_name):
        self.tbl_name = tbl_name
    def get_last_one(self):
        entry = DB_ENTRY()
        db_con = MySQLdb.connect("localhost","root","ct","ct_db" )
        db_cursor = db_con.cursor()
        try:
            sql = "SELECT * FROM " + self.tbl_name +" ORDER BY time DESC LIMIT 1"
            db_cursor.execute(sql)
            results = db_cursor.fetchall()
            for row in results:
                entry.time = int(row[0])
                entry.value = float(row[1])
        except:
            print "error"
        db_con.close()
        return entry
