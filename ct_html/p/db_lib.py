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
        
    def get_all_data_by_date(self):
        data_list = {}
        db_con = MySQLdb.connect("localhost","root","ct","ct_db" )
        db_cursor = db_con.cursor()
        try:
            sql = "SELECT FLOOR(time/100) date,avg(value) FROM" + self.tbl_name +" GROUP BY date ORDER BY date"
            db_cursor.execute(sql)
            results = db_cursor.fetchall()
            for row in results:
                data_list[int(row[0])] = float(row[1])
        except:
            print "error"
        db_con.close()
        return data_list

def cal_percent_str(value, cmp_value):
    per_diff = (value - cmp_value)/((value + cmp_value)/2)
    str = ".2f%" % (per_diff*100)
    return str