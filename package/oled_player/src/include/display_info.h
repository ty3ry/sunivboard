/*
   Copyright (c) 2018, Adrian Rossiter

   Antiprism - http://www.antiprism.com

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

      The above copyright notice and this permission notice shall be included
      in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#ifndef DISPLAY_INFO_H
#define DISPLAY_INFO_H

// #include "status.h" /*c_e: disable*/
#include <string>
#include "timer.h"
#include <vector>

class connection_info {
private:
  std::string if_name;
  std::string ip_addr = "192.168.1.101";
  int type;
  int link;

public:
  enum { TYPE_ETH = 0, TYPE_WIFI, TYPE_UNKNOWN };

  connection_info() : type(TYPE_UNKNOWN) {}
  bool init();
  bool is_set() const { return type != TYPE_UNKNOWN; }
  std::string get_if_name() const { return if_name; }
  std::string get_ip_addr() const { return ip_addr; }
  int get_type() const { return (int)type; }
  int get_link() const { return link; }
};

struct spect_graph {
  int gap;                            // size of gap in pixels
  std::vector<unsigned char> heights; // bar heights

  void init(int bars, int gap_sz)
  {
    gap = gap_sz;
    heights.resize(bars, 0);
  }
};

class mpd_info {
private:
  std::string _title;
  std::string _origin;
public:
  std::string get_title() const { return _title; }
  std::string get_origin() const { return _origin; }
  void set_title(std::string title) { _title = title; }
  void set_origin(std::string origin) { _origin = origin; }
};

struct display_info {
  spect_graph spect;
  mpd_info status; /*c_e: disable*/
  Counter text_change;
  std::vector<double> scroll;
  int clock_format;
  int date_format;
  char pause_screen;
  connection_info conn; /*c_e: disable*/
  //void conn_init() { conn.init(); } /*c_e: disable*/
  void update_from(const display_info &new_info);
};

inline void display_info::update_from(const display_info &new_info)
{
  /*c_e: disable*/
  // bool changed = (status.get_title() != new_info.status.get_title() ||
  //                 status.get_origin() != new_info.status.get_origin() ||
  //                 status.get_state() != new_info.status.get_state());
  bool changed = 0;
  *this = new_info;
  if (changed)
    text_change.reset();
}

#endif // DISPLAY_INFO_H
