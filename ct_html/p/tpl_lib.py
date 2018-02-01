#!/usr/bin/python2.7 

def td_diff_pct(color_type, value) :
    str = ""
    if color_type == 0:
        str += "    <td>"
    elif color_type == 1:
        if value > 0:
            str += "    <td style=\"color:red\">"
        elif value < 0:
            str += "    <td style=\"color:green\">"
        else :
            str += "    <td>"

    if value > 0:
        str += "+%.2f%%" % (100*value)
    else :
        str += "%.2f%%" % (100*value)

    str += "</td>"
    return str
    
def td_occ_pct(color_type, value, low_warn, high_warn) :
    str = ""
    if color_type == 0:
        str += "    <td>"
    elif color_type == 1:
        if value > high_warn:
            str += "    <td style=\"color:red\">"
        elif value < low_warn:
            str += "    <td style=\"color:green\">"
        else :
            str += "    <td>"

    str += "%.2f%%</td>" % (100*value)

    return str