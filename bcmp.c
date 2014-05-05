<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<meta name="robots" content="noindex,nofollow" />
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<meta name="generator" content="0.12-dev (83a0914)" />
<meta http-equiv="X-UA-Compatible" content="IE=8" />
<link rel="icon" href="/source/default/img/icon.png" type="image/png" />
<link rel="stylesheet" type="text/css" media="all"
    title="Default" href="/source/default/style.css" />
<link rel="alternate stylesheet" type="text/css" media="all"
    title="Paper White" href="/source/default/print.css" />
<link rel="stylesheet" type="text/css" href="/source/default/print.css" media="print" />
<link rel="stylesheet" type="text/css" href="/source/default/jquery.tooltip.css" />

<link rel="search" href="/source/opensearch"
    type="application/opensearchdescription+xml"
    title="OpenGrok Search for current project(s)" />
<script type="text/javascript" src="/source/jquery-1.4.4.min.js"></script>
<script type="text/javascript" src="/source/jquery.tooltip-1.3.pack.js"></script>

<script type="text/javascript" src="/source/utils.js"></script>
<title>Cross Reference: /illumos-gate/usr/src/lib/libbc/libc/gen/common/bcmp.c</title>
</head><body>
<script type="text/javascript">/* <![CDATA[ */
    document.hash = 'null';document.rev = '';document.link = '/source/xref/illumos-gate/usr/src/lib/libbc/libc/gen/common/bcmp.c';document.annotate = false;
    document.domReady.push(function() {domReadyMast();});
    document.pageReady.push(function() { pageReadyMast();});
/* ]]> */</script>
<div id="page">
    <div id="whole_header">
        <form action="/source/search">
<div id="header">
<a href="/source/"><span id="MastheadLogo"></span></a>



    <div id="pagetitle"><span id="filename"
                    >Cross Reference: bcmp.c</span></div>
</div>
<div id="Masthead">
    <tt><a href="/source/xref/">xref</a>: /<a href="/source/xref/illumos-gate/">illumos-gate</a>/<a href="/source/xref/illumos-gate/usr/">usr</a>/<a href="/source/xref/illumos-gate/usr/src/">src</a>/<a href="/source/xref/illumos-gate/usr/src/lib/">lib</a>/<a href="/source/xref/illumos-gate/usr/src/lib/libbc/">libbc</a>/<a href="/source/xref/illumos-gate/usr/src/lib/libbc/libc/">libc</a>/<a href="/source/xref/illumos-gate/usr/src/lib/libbc/libc/gen/">gen</a>/<a href="/source/xref/illumos-gate/usr/src/lib/libbc/libc/gen/common/">common</a>/<a href="/source/xref/illumos-gate/usr/src/lib/libbc/libc/gen/common/bcmp.c">bcmp.c</a></tt>
</div>
<div id="bar">
    <ul>
        <li><a href="/source/"><span id="home"></span>Home</a></li><li><a href="/source/history/illumos-gate/usr/src/lib/libbc/libc/gen/common/bcmp.c"><span id="history"></span>History</a></li><li><a href="#" onclick="javascript:get_annotations(); return false;"
            ><span class="annotate"></span>Annotate</a></li><li><a href="#" onclick="javascript:lntoggle();return false;"
            title="Show or hide line numbers (might be slower if file has more than 10 000 lines)."><span id="line"></span>Line#</a></li><li><a
            href="#" onclick="javascript:lsttoggle();return false;"
            title="Show or hide symbol list."><span id="defbox"></span>Navigate</a></li><li><a href="/source/raw/illumos-gate/usr/src/lib/libbc/libc/gen/common/bcmp.c"><span id="download"></span>Download</a></li><li><input type="text" id="search" name="q" class="q" />
            <input type="submit" value="Search" class="submit" /></li><li><input type="checkbox" name="path" value="/illumos-gate/usr/src/lib/libbc/libc/gen/common/" /> only in <b>/illumos-gate/usr/src/lib/libbc/libc/gen/common/</b></li>
    </ul>
    <input type="hidden" name="project" value="illumos-gate" />
</div>
        </form>
    </div>
<div id="content">

<script type="text/javascript">/* <![CDATA[ */
document.pageReady.push(function() { pageReadyList();});
/* ]]> */</script>

<div id="src">
    <pre><script type="text/javascript">/* <![CDATA[ */
function get_sym_list(){return [["Function","xf",[["bcmp",30]]]];} /* ]]> */</script><a class="l" name="1" href="#1">1</a><span class="c">/*
<a class="l" name="2" href="#2">2</a> * CDDL HEADER START
<a class="l" name="3" href="#3">3</a> *
<a class="l" name="4" href="#4">4</a> * The contents of this file are subject to the terms of the
<a class="l" name="5" href="#5">5</a> * Common Development and Distribution License, Version 1.0 only
<a class="l" name="6" href="#6">6</a> * (the "License").  You may not use this file except in compliance
<a class="l" name="7" href="#7">7</a> * with the License.
<a class="l" name="8" href="#8">8</a> *
<a class="l" name="9" href="#9">9</a> * You can obtain a copy of the license at <a href="/source/s?path=usr/">usr</a>/<a href="/source/s?path=usr/src/">src</a>/<a href="/source/s?path=usr/src/OPENSOLARIS.LICENSE">OPENSOLARIS.LICENSE</a>
<a class="hl" name="10" href="#10">10</a> * or <a href="http://www.opensolaris.org/os/licensing">http://www.opensolaris.org/os/licensing</a>.
<a class="l" name="11" href="#11">11</a> * See the License for the specific language governing permissions
<a class="l" name="12" href="#12">12</a> * and limitations under the License.
<a class="l" name="13" href="#13">13</a> *
<a class="l" name="14" href="#14">14</a> * When distributing Covered Code, include this CDDL HEADER in each
<a class="l" name="15" href="#15">15</a> * file and include the License file at <a href="/source/s?path=usr/">usr</a>/<a href="/source/s?path=usr/src/">src</a>/<a href="/source/s?path=usr/src/OPENSOLARIS.LICENSE">OPENSOLARIS.LICENSE</a>.
<a class="l" name="16" href="#16">16</a> * If applicable, add the following below this CDDL HEADER, with the
<a class="l" name="17" href="#17">17</a> * fields enclosed by brackets "[]" replaced with your own identifying
<a class="l" name="18" href="#18">18</a> * information: Portions Copyright [yyyy] [name of copyright owner]
<a class="l" name="19" href="#19">19</a> *
<a class="hl" name="20" href="#20">20</a> * CDDL HEADER END
<a class="l" name="21" href="#21">21</a> */</span>
<a class="l" name="22" href="#22">22</a><span class="c">/*
<a class="l" name="23" href="#23">23</a> * Copyright 1990 Sun Microsystems, Inc.  All rights reserved.
<a class="l" name="24" href="#24">24</a> * Use is subject to license terms.
<a class="l" name="25" href="#25">25</a> */</span>
<a class="l" name="26" href="#26">26</a>
<a class="l" name="27" href="#27">27</a>#<b>pragma</b> <b>ident</b>	<span class="s">"%Z%%M%	%I%	%E% SMI"</span>
<a class="l" name="28" href="#28">28</a>
<a class="l" name="29" href="#29">29</a><b>int</b>
<a class="hl" name="30" href="#30">30</a><a class="xf" name="bcmp"/><a href="/source/s?refs=bcmp&amp;project=illumos-gate" class="xf">bcmp</a>(<b>char</b> *<a class="xa" name="s1"/><a href="/source/s?refs=s1&amp;project=illumos-gate" class="xa">s1</a>, <b>char</b> *<a class="xa" name="s2"/><a href="/source/s?refs=s2&amp;project=illumos-gate" class="xa">s2</a>, <b>int</b> <a class="xa" name="len"/><a href="/source/s?refs=len&amp;project=illumos-gate" class="xa">len</a>)
<a class="l" name="31" href="#31">31</a>{
<a class="l" name="32" href="#32">32</a>
<a class="l" name="33" href="#33">33</a>	<b>while</b> (<a class="d" href="#len">len</a>--)
<a class="l" name="34" href="#34">34</a>		<b>if</b> (*<a class="d" href="#s1">s1</a>++ != *<a class="d" href="#s2">s2</a>++)
<a class="l" name="35" href="#35">35</a>			<b>return</b> (<span class="n">1</span>);
<a class="l" name="36" href="#36">36</a>	<b>return</b> (0);
<a class="l" name="37" href="#37">37</a>}
<a class="l" name="38" href="#38">38</a></pre>
</div>
    <div id="footer">
<p><a href="http://www.opensolaris.org/os/project/opengrok/"
 title="Served by OpenGrok"><span id="fti"></span></a></p>
<p>Indexes created Thu May 01 03:06:45 UTC 2014</p>
    
    </div>
    </div>
</div>
</body>
</html>


