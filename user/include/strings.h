












<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<!-- ViewVC :: http://www.viewvc.org/ -->
<head>
<title>[base] Contents of /head/include/strings.h</title>
<meta name="generator" content="ViewVC 1.1.22" />
<link rel="shortcut icon" href="/*docroot*/images/favicon.ico" />
<link rel="stylesheet" href="/*docroot*/styles.css" type="text/css" />

</head>
<body>
<div class="vc_navheader">
<table><tr>
<td><strong><a href="/"><span class="pathdiv">/</span></a><a href="/base/">[base]</a><span class="pathdiv">/</span><a href="/base/head/">head</a><span class="pathdiv">/</span><a href="/base/head/include/">include</a><span class="pathdiv">/</span><a href="/base/head/include/strings.h?view=log">strings.h</a></strong></td>
<td style="text-align: right;"></td>
</tr></table>
</div>
<div style="float: right; padding: 5px;"><a href="http://www.viewvc.org/" title="ViewVC Home"><img src="/*docroot*/images/viewvc-logo.png" alt="ViewVC logotype" width="240" height="70" /></a></div>
<h1>Contents of /head/include/strings.h</h1>

<p style="margin:0;">

<a href="/base/head/include/"><img src="/*docroot*/images/back_small.png" class="vc_icon" alt="Parent Directory" /> Parent Directory</a>

| <a href="/base/head/include/strings.h?view=log"><img src="/*docroot*/images/log.png" class="vc_icon" alt="Revision Log" /> Revision Log</a>




</p>

<hr />
<div class="vc_summary">
Revision <a href="/base?view=revision&amp;revision=266865"><strong>266865</strong></a> -
(<a href="/base/head/include/strings.h?annotate=266865"><strong>show annotations</strong></a>)
(<a href="/base/head/include/strings.h?revision=266865&amp;view=co"><strong>download</strong></a>)


<br /><em>Fri May 30 01:09:07 2014 UTC</em>
(4 months, 1 week ago)
by <em>pfg</em>






<br />File MIME type: text/plain


<br />File size: 2416 byte(s)






<pre class="vc_log">Fix strcasecmp_l() and strncasecmp_l() POSIX 2008 compliance.

POSIX.1-2008 specifies that those two functions should be declared by
including &lt;strings.h&gt;, not &lt;string.h&gt; (the latter only has strcoll_l()
and strxfrm_l()):

<a href="http://pubs.opengroup.org/onlinepubs/9699919799/functions/strcasecmp.html">http://pubs.opengroup.org/onlinepubs/9699919799/functions/strcasecmp.html</a>

Obtained from:	DragonFlyBSD
Reviewed by:	theraven
MFC after:	2 weeks

</pre>

</div>






<div id="vc_file">
<table cellspacing="0" cellpadding="0">








<tr class="vc_row_odd" id="l1">
<td class="vc_file_line_number"><a href="#l1">1</a></td>

<td class="vc_file_line_text">/*-</td>
</tr>




<tr class="vc_row_odd" id="l2">
<td class="vc_file_line_number"><a href="#l2">2</a></td>

<td class="vc_file_line_text"> * Copyright (c) 2002 Mike Barcroft &lt;mike@FreeBSD.org&gt;</td>
</tr>




<tr class="vc_row_odd" id="l3">
<td class="vc_file_line_number"><a href="#l3">3</a></td>

<td class="vc_file_line_text"> * All rights reserved.</td>
</tr>




<tr class="vc_row_odd" id="l4">
<td class="vc_file_line_number"><a href="#l4">4</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l5">
<td class="vc_file_line_number"><a href="#l5">5</a></td>

<td class="vc_file_line_text"> * Redistribution and use in source and binary forms, with or without</td>
</tr>




<tr class="vc_row_odd" id="l6">
<td class="vc_file_line_number"><a href="#l6">6</a></td>

<td class="vc_file_line_text"> * modification, are permitted provided that the following conditions</td>
</tr>




<tr class="vc_row_odd" id="l7">
<td class="vc_file_line_number"><a href="#l7">7</a></td>

<td class="vc_file_line_text"> * are met:</td>
</tr>




<tr class="vc_row_odd" id="l8">
<td class="vc_file_line_number"><a href="#l8">8</a></td>

<td class="vc_file_line_text"> * 1. Redistributions of source code must retain the above copyright</td>
</tr>




<tr class="vc_row_odd" id="l9">
<td class="vc_file_line_number"><a href="#l9">9</a></td>

<td class="vc_file_line_text"> *    notice, this list of conditions and the following disclaimer.</td>
</tr>




<tr class="vc_row_odd" id="l10">
<td class="vc_file_line_number"><a href="#l10">10</a></td>

<td class="vc_file_line_text"> * 2. Redistributions in binary form must reproduce the above copyright</td>
</tr>




<tr class="vc_row_odd" id="l11">
<td class="vc_file_line_number"><a href="#l11">11</a></td>

<td class="vc_file_line_text"> *    notice, this list of conditions and the following disclaimer in the</td>
</tr>




<tr class="vc_row_odd" id="l12">
<td class="vc_file_line_number"><a href="#l12">12</a></td>

<td class="vc_file_line_text"> *    documentation and/or other materials provided with the distribution.</td>
</tr>




<tr class="vc_row_odd" id="l13">
<td class="vc_file_line_number"><a href="#l13">13</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l14">
<td class="vc_file_line_number"><a href="#l14">14</a></td>

<td class="vc_file_line_text"> * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS&#39;&#39; AND</td>
</tr>




<tr class="vc_row_odd" id="l15">
<td class="vc_file_line_number"><a href="#l15">15</a></td>

<td class="vc_file_line_text"> * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE</td>
</tr>




<tr class="vc_row_odd" id="l16">
<td class="vc_file_line_number"><a href="#l16">16</a></td>

<td class="vc_file_line_text"> * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE</td>
</tr>




<tr class="vc_row_odd" id="l17">
<td class="vc_file_line_number"><a href="#l17">17</a></td>

<td class="vc_file_line_text"> * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE</td>
</tr>




<tr class="vc_row_odd" id="l18">
<td class="vc_file_line_number"><a href="#l18">18</a></td>

<td class="vc_file_line_text"> * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL</td>
</tr>




<tr class="vc_row_odd" id="l19">
<td class="vc_file_line_number"><a href="#l19">19</a></td>

<td class="vc_file_line_text"> * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS</td>
</tr>




<tr class="vc_row_odd" id="l20">
<td class="vc_file_line_number"><a href="#l20">20</a></td>

<td class="vc_file_line_text"> * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)</td>
</tr>




<tr class="vc_row_odd" id="l21">
<td class="vc_file_line_number"><a href="#l21">21</a></td>

<td class="vc_file_line_text"> * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT</td>
</tr>




<tr class="vc_row_odd" id="l22">
<td class="vc_file_line_number"><a href="#l22">22</a></td>

<td class="vc_file_line_text"> * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY</td>
</tr>




<tr class="vc_row_odd" id="l23">
<td class="vc_file_line_number"><a href="#l23">23</a></td>

<td class="vc_file_line_text"> * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF</td>
</tr>




<tr class="vc_row_odd" id="l24">
<td class="vc_file_line_number"><a href="#l24">24</a></td>

<td class="vc_file_line_text"> * SUCH DAMAGE.</td>
</tr>




<tr class="vc_row_odd" id="l25">
<td class="vc_file_line_number"><a href="#l25">25</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l26">
<td class="vc_file_line_number"><a href="#l26">26</a></td>

<td class="vc_file_line_text"> * $FreeBSD$</td>
</tr>




<tr class="vc_row_odd" id="l27">
<td class="vc_file_line_number"><a href="#l27">27</a></td>

<td class="vc_file_line_text"> */</td>
</tr>




<tr class="vc_row_odd" id="l28">
<td class="vc_file_line_number"><a href="#l28">28</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l29">
<td class="vc_file_line_number"><a href="#l29">29</a></td>

<td class="vc_file_line_text">#ifndef _STRINGS_H_</td>
</tr>




<tr class="vc_row_odd" id="l30">
<td class="vc_file_line_number"><a href="#l30">30</a></td>

<td class="vc_file_line_text">#define _STRINGS_H_</td>
</tr>




<tr class="vc_row_odd" id="l31">
<td class="vc_file_line_number"><a href="#l31">31</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l32">
<td class="vc_file_line_number"><a href="#l32">32</a></td>

<td class="vc_file_line_text">#include &lt;sys/cdefs.h&gt;</td>
</tr>




<tr class="vc_row_odd" id="l33">
<td class="vc_file_line_number"><a href="#l33">33</a></td>

<td class="vc_file_line_text">#include &lt;sys/_types.h&gt;</td>
</tr>




<tr class="vc_row_odd" id="l34">
<td class="vc_file_line_number"><a href="#l34">34</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l35">
<td class="vc_file_line_number"><a href="#l35">35</a></td>

<td class="vc_file_line_text">#ifndef _SIZE_T_DECLARED</td>
</tr>




<tr class="vc_row_odd" id="l36">
<td class="vc_file_line_number"><a href="#l36">36</a></td>

<td class="vc_file_line_text">typedef __size_t        size_t;</td>
</tr>




<tr class="vc_row_odd" id="l37">
<td class="vc_file_line_number"><a href="#l37">37</a></td>

<td class="vc_file_line_text">#define _SIZE_T_DECLARED</td>
</tr>




<tr class="vc_row_odd" id="l38">
<td class="vc_file_line_number"><a href="#l38">38</a></td>

<td class="vc_file_line_text">#endif</td>
</tr>




<tr class="vc_row_odd" id="l39">
<td class="vc_file_line_number"><a href="#l39">39</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l40">
<td class="vc_file_line_number"><a href="#l40">40</a></td>

<td class="vc_file_line_text">__BEGIN_DECLS</td>
</tr>




<tr class="vc_row_odd" id="l41">
<td class="vc_file_line_number"><a href="#l41">41</a></td>

<td class="vc_file_line_text">#if __BSD_VISIBLE || __POSIX_VISIBLE &lt;= 200112</td>
</tr>




<tr class="vc_row_odd" id="l42">
<td class="vc_file_line_number"><a href="#l42">42</a></td>

<td class="vc_file_line_text">int      bcmp(const void *, const void *, size_t) __pure;       /* LEGACY */</td>
</tr>




<tr class="vc_row_odd" id="l43">
<td class="vc_file_line_number"><a href="#l43">43</a></td>

<td class="vc_file_line_text">void     bcopy(const void *, void *, size_t);                   /* LEGACY */</td>
</tr>




<tr class="vc_row_odd" id="l44">
<td class="vc_file_line_number"><a href="#l44">44</a></td>

<td class="vc_file_line_text">void     bzero(void *, size_t);                                 /* LEGACY */</td>
</tr>




<tr class="vc_row_odd" id="l45">
<td class="vc_file_line_number"><a href="#l45">45</a></td>

<td class="vc_file_line_text">#endif</td>
</tr>




<tr class="vc_row_odd" id="l46">
<td class="vc_file_line_number"><a href="#l46">46</a></td>

<td class="vc_file_line_text">#if __XSI_VISIBLE</td>
</tr>




<tr class="vc_row_odd" id="l47">
<td class="vc_file_line_number"><a href="#l47">47</a></td>

<td class="vc_file_line_text">int      ffs(int) __pure2;</td>
</tr>




<tr class="vc_row_odd" id="l48">
<td class="vc_file_line_number"><a href="#l48">48</a></td>

<td class="vc_file_line_text">#endif</td>
</tr>




<tr class="vc_row_odd" id="l49">
<td class="vc_file_line_number"><a href="#l49">49</a></td>

<td class="vc_file_line_text">#if __BSD_VISIBLE</td>
</tr>




<tr class="vc_row_odd" id="l50">
<td class="vc_file_line_number"><a href="#l50">50</a></td>

<td class="vc_file_line_text">int      ffsl(long) __pure2;</td>
</tr>




<tr class="vc_row_odd" id="l51">
<td class="vc_file_line_number"><a href="#l51">51</a></td>

<td class="vc_file_line_text">int      ffsll(long long) __pure2;</td>
</tr>




<tr class="vc_row_odd" id="l52">
<td class="vc_file_line_number"><a href="#l52">52</a></td>

<td class="vc_file_line_text">int      fls(int) __pure2;</td>
</tr>




<tr class="vc_row_odd" id="l53">
<td class="vc_file_line_number"><a href="#l53">53</a></td>

<td class="vc_file_line_text">int      flsl(long) __pure2;</td>
</tr>




<tr class="vc_row_odd" id="l54">
<td class="vc_file_line_number"><a href="#l54">54</a></td>

<td class="vc_file_line_text">int      flsll(long long) __pure2;</td>
</tr>




<tr class="vc_row_odd" id="l55">
<td class="vc_file_line_number"><a href="#l55">55</a></td>

<td class="vc_file_line_text">#endif</td>
</tr>




<tr class="vc_row_odd" id="l56">
<td class="vc_file_line_number"><a href="#l56">56</a></td>

<td class="vc_file_line_text">#if __BSD_VISIBLE || __POSIX_VISIBLE &lt;= 200112</td>
</tr>




<tr class="vc_row_odd" id="l57">
<td class="vc_file_line_number"><a href="#l57">57</a></td>

<td class="vc_file_line_text">char    *index(const char *, int) __pure;                       /* LEGACY */</td>
</tr>




<tr class="vc_row_odd" id="l58">
<td class="vc_file_line_number"><a href="#l58">58</a></td>

<td class="vc_file_line_text">char    *rindex(const char *, int) __pure;                      /* LEGACY */</td>
</tr>




<tr class="vc_row_odd" id="l59">
<td class="vc_file_line_number"><a href="#l59">59</a></td>

<td class="vc_file_line_text">#endif</td>
</tr>




<tr class="vc_row_odd" id="l60">
<td class="vc_file_line_number"><a href="#l60">60</a></td>

<td class="vc_file_line_text">int      strcasecmp(const char *, const char *) __pure;</td>
</tr>




<tr class="vc_row_odd" id="l61">
<td class="vc_file_line_number"><a href="#l61">61</a></td>

<td class="vc_file_line_text">int      strncasecmp(const char *, const char *, size_t) __pure;</td>
</tr>




<tr class="vc_row_odd" id="l62">
<td class="vc_file_line_number"><a href="#l62">62</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l63">
<td class="vc_file_line_number"><a href="#l63">63</a></td>

<td class="vc_file_line_text">#if __POSIX_VISIBLE &gt;= 200809 || defined(_XLOCALE_H_)</td>
</tr>




<tr class="vc_row_odd" id="l64">
<td class="vc_file_line_number"><a href="#l64">64</a></td>

<td class="vc_file_line_text">#include &lt;xlocale/_strings.h&gt;</td>
</tr>




<tr class="vc_row_odd" id="l65">
<td class="vc_file_line_number"><a href="#l65">65</a></td>

<td class="vc_file_line_text">#endif</td>
</tr>




<tr class="vc_row_odd" id="l66">
<td class="vc_file_line_number"><a href="#l66">66</a></td>

<td class="vc_file_line_text">__END_DECLS</td>
</tr>




<tr class="vc_row_odd" id="l67">
<td class="vc_file_line_number"><a href="#l67">67</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l68">
<td class="vc_file_line_number"><a href="#l68">68</a></td>

<td class="vc_file_line_text">#endif /* _STRINGS_H_ */</td>
</tr>


</table>
</div>



<hr/>
<div class="vc_properties">
<h2>Properties</h2>
<table cellspacing="1" cellpadding="2" class="auto">
<thead>
<tr>
<th class="vc_header_sort">Name</th>
<th class="vc_header">Value</th>
</tr>
</thead>
<tbody>

<tr class="vc_row_even">
<td><strong>svn:keywords</strong></td>

<td style="white-space: pre;">FreeBSD=%H
</td>

</tr>

</tbody>
</table>
</div>



<hr />
<table>
<tr>
<td>&nbsp;</td>
<td style="text-align: right;"><strong><a href="/*docroot*/help_rootview.html">ViewVC Help</a></strong></td>
</tr>
<tr>
<td>Powered by <a href="http://viewvc.tigris.org/">ViewVC 1.1.22</a></td>
<td style="text-align: right;">&nbsp;</td>
</tr>
</table>
</body>
</html>

