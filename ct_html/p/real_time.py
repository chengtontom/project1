#!/usr/bin/python2.7 
import db_lib

tbl_name_arr = ('tbl_ex_us_euro','tbl_ex_us_uk','tbl_ex_us_jpn','tbl_ex_us_can','tbl_ex_us_chn',\
               'tbl_au_g_chn','tbl_ag_g_chn','tbl_brent_crude_us','tbl_ns_gas_us','tbl_me_au_us',\
               'tbl_me_ag_us','tbl_me_pt_us','tbl_me_cu_us','tbl_mk_dow','tbl_mk_sp','tbl_mk_ftse',\
               'tbl_mk_cac','tbl_mk_dax','tbl_mk_hs','tbl_mk_nk','tbl_mk_sse',)

def html_name(name) :
    if name == 'tbl_ex_us_euro':
        return "US - EURO"
    elif name == 'tbl_ex_us_uk':
        return "US - UK"
    elif name == 'tbl_ex_us_jpn':
        return "US - JPN"
    elif name == 'tbl_ex_us_can':
        return "US - CAN"
    elif name == 'tbl_ex_us_chn':
        return "US - CHN"
    elif name == 'tbl_au_g_chn':
        return "AU - CHN"
    elif name == 'tbl_ag_g_chn':
        return "AG - CHN"
    elif name == 'tbl_brent_crude_us':
        return "BRENT CURDE - US"
    elif name == 'tbl_ns_gas_us':
        return "NATURAL GAS - US"
    elif name == 'tbl_me_au_us':
        return "AU - US"
    elif name == 'tbl_me_ag_us':
        return "AG - US"
    elif name == 'tbl_me_pt_us':
        return "PT - US"
    elif name == 'tbl_me_cu_us':
        return "CU - US"
    elif name == 'tbl_mk_dow':
        return "DOW JONES"
    elif name == 'tbl_mk_sp':
        return "SP 500"
    elif name == 'tbl_mk_ftse':
        return "FTSE"
    elif name == 'tbl_mk_cac':
        return "CAC"
    elif name == 'tbl_mk_dax':
        return "DAX"
    elif name == 'tbl_mk_hs':
        return "HANG SENG"
    elif name == 'tbl_mk_nk':
        return "NIKKEI"
    elif name == 'tbl_mk_sse':
        return "SSE"


def show() :
    print "<h2>real-time</h2>"
    print "<table border=\"1\">"
    print "<tr>"
    print "<th>Type</th>"
    print "<th>Time</th>"
    print "<th>Value</th>"
    print "<th>Cmp Day</th>"
    print "<th>Cmp Week</th>"
    print "<th>Low Week PCT</th>"
    print "<th>Low Month PCT</th>"
    print "<th>Low Season PCT</th>"
    print "<th>Low All PCT</th>"
    print "</tr>"
    for tbl_name in tbl_name_arr :
        db_tbl = db_lib.DB_TBL(tbl_name)
        entry = db_tbl.get_last_one()
        h_name = html_name(tbl_name)
        db_date_list = db_tbl.get_all_data_by_date();
        old_date = db_tbl.get_oldest_date();
        
        print "<tr>"
        print "    <td>%s</td> <td>%d</td> <td>%.3f</td>" % (h_name,entry.time,entry.value)

        print "    <td>%s</td>" % (db_lib.cal_diff_percent_str(entry.value, db_tbl.get_value_by_date(entry.date-1)))
        print "    <td>%s</td>" % (db_lib.cal_diff_percent_str(entry.value, db_tbl.get_value_by_date(entry.date-7)))
        print "    <td>%s</td>" % (db_lib.cal_diff_percent_str(entry.value, db_tbl.get_value_by_date(entry.date-30)))

        print "    <td>%s</td>" % (db_lib.cal_low_list_percent_str(db_date_list, entry.value, 7))
        print "    <td>%s</td>" % (db_lib.cal_low_list_percent_str(db_date_list, entry.value, 30))
        print "    <td>%s</td>" % (db_lib.cal_low_list_percent_str(db_date_list, entry.value, 90))
        print "    <td>%s</td>" % (db_lib.cal_low_list_percent_str(db_date_list, entry.value, 0))
        
        print "</tr>"
    print "</table>"

# show()
