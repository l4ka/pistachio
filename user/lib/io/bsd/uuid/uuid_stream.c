












<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<!-- ViewVC :: http://www.viewvc.org/ -->
<head>
<title>[base] Contents of /head/lib/libc/uuid/uuid_stream.c</title>
<meta name="generator" content="ViewVC 1.1.22" />
<link rel="shortcut icon" href="/*docroot*/images/favicon.ico" />
<link rel="stylesheet" href="/*docroot*/styles.css" type="text/css" />

</head>
<body>
<div class="vc_navheader">
<table><tr>
<td><strong><a href="/"><span class="pathdiv">/</span></a><a href="/base/">[base]</a><span class="pathdiv">/</span><a href="/base/head/">head</a><span class="pathdiv">/</span><a href="/base/head/lib/">lib</a><span class="pathdiv">/</span><a href="/base/head/lib/libc/">libc</a><span class="pathdiv">/</span><a href="/base/head/lib/libc/uuid/">uuid</a><span class="pathdiv">/</span><a href="/base/head/lib/libc/uuid/uuid_stream.c?view=log">uuid_stream.c</a></strong></td>
<td style="text-align: right;"></td>
</tr></table>
</div>
<div style="float: right; padding: 5px;"><a href="http://www.viewvc.org/" title="ViewVC Home"><img src="/*docroot*/images/viewvc-logo.png" alt="ViewVC logotype" width="240" height="70" /></a></div>
<h1>Contents of /head/lib/libc/uuid/uuid_stream.c</h1>

<p style="margin:0;">

<a href="/base/head/lib/libc/uuid/"><img src="/*docroot*/images/back_small.png" class="vc_icon" alt="Parent Directory" /> Parent Directory</a>

| <a href="/base/head/lib/libc/uuid/uuid_stream.c?view=log"><img src="/*docroot*/images/log.png" class="vc_icon" alt="Revision Log" /> Revision Log</a>




</p>

<hr />
<div class="vc_summary">
Revision <a href="/base?view=revision&amp;revision=181743"><strong>181743</strong></a> -
(<a href="/base/head/lib/libc/uuid/uuid_stream.c?annotate=181743"><strong>show annotations</strong></a>)
(<a href="/base/head/lib/libc/uuid/uuid_stream.c?revision=181743&amp;view=co"><strong>download</strong></a>)


<br /><em>Thu Aug 14 22:23:16 2008 UTC</em>
(6 years, 1 month ago)
by <em>emax</em>






<br />File MIME type: text/plain


<br />File size: 3781 byte(s)






<pre class="vc_log">Import the uuid_enc_le(), uuid_dec_le(), uuid_enc_be() and
uuid_dec_be() functions. These routines are not part of the
DCE RPC API. They are provided for convenience.

Reviewed by:	marcel
Obtained from:	NetBSD
MFC after:	1 week

</pre>

</div>






<div id="vc_file">
<table cellspacing="0" cellpadding="0">








<tr class="vc_row_odd" id="l1">
<td class="vc_file_line_number"><a href="#l1">1</a></td>

<td class="vc_file_line_text">/*      $NetBSD: uuid_stream.c,v 1.3 2008/04/19 18:21:38 plunky Exp $   */</td>
</tr>




<tr class="vc_row_odd" id="l2">
<td class="vc_file_line_number"><a href="#l2">2</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l3">
<td class="vc_file_line_number"><a href="#l3">3</a></td>

<td class="vc_file_line_text">/*-</td>
</tr>




<tr class="vc_row_odd" id="l4">
<td class="vc_file_line_number"><a href="#l4">4</a></td>

<td class="vc_file_line_text"> * Copyright (c) 2002 Marcel Moolenaar</td>
</tr>




<tr class="vc_row_odd" id="l5">
<td class="vc_file_line_number"><a href="#l5">5</a></td>

<td class="vc_file_line_text"> * All rights reserved.</td>
</tr>




<tr class="vc_row_odd" id="l6">
<td class="vc_file_line_number"><a href="#l6">6</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l7">
<td class="vc_file_line_number"><a href="#l7">7</a></td>

<td class="vc_file_line_text"> * Redistribution and use in source and binary forms, with or without</td>
</tr>




<tr class="vc_row_odd" id="l8">
<td class="vc_file_line_number"><a href="#l8">8</a></td>

<td class="vc_file_line_text"> * modification, are permitted provided that the following conditions</td>
</tr>




<tr class="vc_row_odd" id="l9">
<td class="vc_file_line_number"><a href="#l9">9</a></td>

<td class="vc_file_line_text"> * are met:</td>
</tr>




<tr class="vc_row_odd" id="l10">
<td class="vc_file_line_number"><a href="#l10">10</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l11">
<td class="vc_file_line_number"><a href="#l11">11</a></td>

<td class="vc_file_line_text"> * 1. Redistributions of source code must retain the above copyright</td>
</tr>




<tr class="vc_row_odd" id="l12">
<td class="vc_file_line_number"><a href="#l12">12</a></td>

<td class="vc_file_line_text"> *    notice, this list of conditions and the following disclaimer.</td>
</tr>




<tr class="vc_row_odd" id="l13">
<td class="vc_file_line_number"><a href="#l13">13</a></td>

<td class="vc_file_line_text"> * 2. Redistributions in binary form must reproduce the above copyright</td>
</tr>




<tr class="vc_row_odd" id="l14">
<td class="vc_file_line_number"><a href="#l14">14</a></td>

<td class="vc_file_line_text"> *    notice, this list of conditions and the following disclaimer in the</td>
</tr>




<tr class="vc_row_odd" id="l15">
<td class="vc_file_line_number"><a href="#l15">15</a></td>

<td class="vc_file_line_text"> *    documentation and/or other materials provided with the distribution.</td>
</tr>




<tr class="vc_row_odd" id="l16">
<td class="vc_file_line_number"><a href="#l16">16</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l17">
<td class="vc_file_line_number"><a href="#l17">17</a></td>

<td class="vc_file_line_text"> * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS&#39;&#39; AND ANY EXPRESS OR</td>
</tr>




<tr class="vc_row_odd" id="l18">
<td class="vc_file_line_number"><a href="#l18">18</a></td>

<td class="vc_file_line_text"> * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES</td>
</tr>




<tr class="vc_row_odd" id="l19">
<td class="vc_file_line_number"><a href="#l19">19</a></td>

<td class="vc_file_line_text"> * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.</td>
</tr>




<tr class="vc_row_odd" id="l20">
<td class="vc_file_line_number"><a href="#l20">20</a></td>

<td class="vc_file_line_text"> * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,</td>
</tr>




<tr class="vc_row_odd" id="l21">
<td class="vc_file_line_number"><a href="#l21">21</a></td>

<td class="vc_file_line_text"> * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT</td>
</tr>




<tr class="vc_row_odd" id="l22">
<td class="vc_file_line_number"><a href="#l22">22</a></td>

<td class="vc_file_line_text"> * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,</td>
</tr>




<tr class="vc_row_odd" id="l23">
<td class="vc_file_line_number"><a href="#l23">23</a></td>

<td class="vc_file_line_text"> * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY</td>
</tr>




<tr class="vc_row_odd" id="l24">
<td class="vc_file_line_number"><a href="#l24">24</a></td>

<td class="vc_file_line_text"> * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT</td>
</tr>




<tr class="vc_row_odd" id="l25">
<td class="vc_file_line_number"><a href="#l25">25</a></td>

<td class="vc_file_line_text"> * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF</td>
</tr>




<tr class="vc_row_odd" id="l26">
<td class="vc_file_line_number"><a href="#l26">26</a></td>

<td class="vc_file_line_text"> * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</td>
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

<td class="vc_file_line_text">#include &lt;sys/cdefs.h&gt;</td>
</tr>




<tr class="vc_row_odd" id="l30">
<td class="vc_file_line_number"><a href="#l30">30</a></td>

<td class="vc_file_line_text">__FBSDID(&quot;$FreeBSD$&quot;);</td>
</tr>




<tr class="vc_row_odd" id="l31">
<td class="vc_file_line_number"><a href="#l31">31</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l32">
<td class="vc_file_line_number"><a href="#l32">32</a></td>

<td class="vc_file_line_text">#include &lt;sys/endian.h&gt;</td>
</tr>




<tr class="vc_row_odd" id="l33">
<td class="vc_file_line_number"><a href="#l33">33</a></td>

<td class="vc_file_line_text">#include &lt;uuid.h&gt;</td>
</tr>




<tr class="vc_row_odd" id="l34">
<td class="vc_file_line_number"><a href="#l34">34</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l35">
<td class="vc_file_line_number"><a href="#l35">35</a></td>

<td class="vc_file_line_text">/*</td>
</tr>




<tr class="vc_row_odd" id="l36">
<td class="vc_file_line_number"><a href="#l36">36</a></td>

<td class="vc_file_line_text"> * Encode/Decode UUID into octet-stream.</td>
</tr>




<tr class="vc_row_odd" id="l37">
<td class="vc_file_line_number"><a href="#l37">37</a></td>

<td class="vc_file_line_text"> *   <a href="http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt">http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt</a></td>
</tr>




<tr class="vc_row_odd" id="l38">
<td class="vc_file_line_number"><a href="#l38">38</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l39">
<td class="vc_file_line_number"><a href="#l39">39</a></td>

<td class="vc_file_line_text"> * 0                   1                   2                   3</td>
</tr>




<tr class="vc_row_odd" id="l40">
<td class="vc_file_line_number"><a href="#l40">40</a></td>

<td class="vc_file_line_text"> *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1</td>
</tr>




<tr class="vc_row_odd" id="l41">
<td class="vc_file_line_number"><a href="#l41">41</a></td>

<td class="vc_file_line_text"> *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+</td>
</tr>




<tr class="vc_row_odd" id="l42">
<td class="vc_file_line_number"><a href="#l42">42</a></td>

<td class="vc_file_line_text"> *  |                          time_low                             |</td>
</tr>




<tr class="vc_row_odd" id="l43">
<td class="vc_file_line_number"><a href="#l43">43</a></td>

<td class="vc_file_line_text"> *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+</td>
</tr>




<tr class="vc_row_odd" id="l44">
<td class="vc_file_line_number"><a href="#l44">44</a></td>

<td class="vc_file_line_text"> *  |       time_mid                |         time_hi_and_version   |</td>
</tr>




<tr class="vc_row_odd" id="l45">
<td class="vc_file_line_number"><a href="#l45">45</a></td>

<td class="vc_file_line_text"> *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+</td>
</tr>




<tr class="vc_row_odd" id="l46">
<td class="vc_file_line_number"><a href="#l46">46</a></td>

<td class="vc_file_line_text"> *  |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |</td>
</tr>




<tr class="vc_row_odd" id="l47">
<td class="vc_file_line_number"><a href="#l47">47</a></td>

<td class="vc_file_line_text"> *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+</td>
</tr>




<tr class="vc_row_odd" id="l48">
<td class="vc_file_line_number"><a href="#l48">48</a></td>

<td class="vc_file_line_text"> *  |                         node (2-5)                            |</td>
</tr>




<tr class="vc_row_odd" id="l49">
<td class="vc_file_line_number"><a href="#l49">49</a></td>

<td class="vc_file_line_text"> *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+</td>
</tr>




<tr class="vc_row_odd" id="l50">
<td class="vc_file_line_number"><a href="#l50">50</a></td>

<td class="vc_file_line_text"> *</td>
</tr>




<tr class="vc_row_odd" id="l51">
<td class="vc_file_line_number"><a href="#l51">51</a></td>

<td class="vc_file_line_text"> * NOTE: These routines are not part of the DCE RPC API. They are</td>
</tr>




<tr class="vc_row_odd" id="l52">
<td class="vc_file_line_number"><a href="#l52">52</a></td>

<td class="vc_file_line_text"> * provided for convenience.</td>
</tr>




<tr class="vc_row_odd" id="l53">
<td class="vc_file_line_number"><a href="#l53">53</a></td>

<td class="vc_file_line_text"> */</td>
</tr>




<tr class="vc_row_odd" id="l54">
<td class="vc_file_line_number"><a href="#l54">54</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l55">
<td class="vc_file_line_number"><a href="#l55">55</a></td>

<td class="vc_file_line_text">void</td>
</tr>




<tr class="vc_row_odd" id="l56">
<td class="vc_file_line_number"><a href="#l56">56</a></td>

<td class="vc_file_line_text">uuid_enc_le(void *buf, const uuid_t *uuid)</td>
</tr>




<tr class="vc_row_odd" id="l57">
<td class="vc_file_line_number"><a href="#l57">57</a></td>

<td class="vc_file_line_text">{</td>
</tr>




<tr class="vc_row_odd" id="l58">
<td class="vc_file_line_number"><a href="#l58">58</a></td>

<td class="vc_file_line_text">        uint8_t *p = buf;</td>
</tr>




<tr class="vc_row_odd" id="l59">
<td class="vc_file_line_number"><a href="#l59">59</a></td>

<td class="vc_file_line_text">        int i;</td>
</tr>




<tr class="vc_row_odd" id="l60">
<td class="vc_file_line_number"><a href="#l60">60</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l61">
<td class="vc_file_line_number"><a href="#l61">61</a></td>

<td class="vc_file_line_text">        le32enc(p, uuid-&gt;time_low);</td>
</tr>




<tr class="vc_row_odd" id="l62">
<td class="vc_file_line_number"><a href="#l62">62</a></td>

<td class="vc_file_line_text">        le16enc(p + 4, uuid-&gt;time_mid);</td>
</tr>




<tr class="vc_row_odd" id="l63">
<td class="vc_file_line_number"><a href="#l63">63</a></td>

<td class="vc_file_line_text">        le16enc(p + 6, uuid-&gt;time_hi_and_version);</td>
</tr>




<tr class="vc_row_odd" id="l64">
<td class="vc_file_line_number"><a href="#l64">64</a></td>

<td class="vc_file_line_text">        p[8] = uuid-&gt;clock_seq_hi_and_reserved;</td>
</tr>




<tr class="vc_row_odd" id="l65">
<td class="vc_file_line_number"><a href="#l65">65</a></td>

<td class="vc_file_line_text">        p[9] = uuid-&gt;clock_seq_low;</td>
</tr>




<tr class="vc_row_odd" id="l66">
<td class="vc_file_line_number"><a href="#l66">66</a></td>

<td class="vc_file_line_text">        for (i = 0; i &lt; _UUID_NODE_LEN; i++)</td>
</tr>




<tr class="vc_row_odd" id="l67">
<td class="vc_file_line_number"><a href="#l67">67</a></td>

<td class="vc_file_line_text">                p[10 + i] = uuid-&gt;node[i];</td>
</tr>




<tr class="vc_row_odd" id="l68">
<td class="vc_file_line_number"><a href="#l68">68</a></td>

<td class="vc_file_line_text">}</td>
</tr>




<tr class="vc_row_odd" id="l69">
<td class="vc_file_line_number"><a href="#l69">69</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l70">
<td class="vc_file_line_number"><a href="#l70">70</a></td>

<td class="vc_file_line_text">void</td>
</tr>




<tr class="vc_row_odd" id="l71">
<td class="vc_file_line_number"><a href="#l71">71</a></td>

<td class="vc_file_line_text">uuid_dec_le(const void *buf, uuid_t *uuid)</td>
</tr>




<tr class="vc_row_odd" id="l72">
<td class="vc_file_line_number"><a href="#l72">72</a></td>

<td class="vc_file_line_text">{</td>
</tr>




<tr class="vc_row_odd" id="l73">
<td class="vc_file_line_number"><a href="#l73">73</a></td>

<td class="vc_file_line_text">        const uint8_t *p = buf;</td>
</tr>




<tr class="vc_row_odd" id="l74">
<td class="vc_file_line_number"><a href="#l74">74</a></td>

<td class="vc_file_line_text">        int i;</td>
</tr>




<tr class="vc_row_odd" id="l75">
<td class="vc_file_line_number"><a href="#l75">75</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l76">
<td class="vc_file_line_number"><a href="#l76">76</a></td>

<td class="vc_file_line_text">        uuid-&gt;time_low = le32dec(p);</td>
</tr>




<tr class="vc_row_odd" id="l77">
<td class="vc_file_line_number"><a href="#l77">77</a></td>

<td class="vc_file_line_text">        uuid-&gt;time_mid = le16dec(p + 4);</td>
</tr>




<tr class="vc_row_odd" id="l78">
<td class="vc_file_line_number"><a href="#l78">78</a></td>

<td class="vc_file_line_text">        uuid-&gt;time_hi_and_version = le16dec(p + 6);</td>
</tr>




<tr class="vc_row_odd" id="l79">
<td class="vc_file_line_number"><a href="#l79">79</a></td>

<td class="vc_file_line_text">        uuid-&gt;clock_seq_hi_and_reserved = p[8];</td>
</tr>




<tr class="vc_row_odd" id="l80">
<td class="vc_file_line_number"><a href="#l80">80</a></td>

<td class="vc_file_line_text">        uuid-&gt;clock_seq_low = p[9];</td>
</tr>




<tr class="vc_row_odd" id="l81">
<td class="vc_file_line_number"><a href="#l81">81</a></td>

<td class="vc_file_line_text">        for (i = 0; i &lt; _UUID_NODE_LEN; i++)</td>
</tr>




<tr class="vc_row_odd" id="l82">
<td class="vc_file_line_number"><a href="#l82">82</a></td>

<td class="vc_file_line_text">                uuid-&gt;node[i] = p[10 + i];</td>
</tr>




<tr class="vc_row_odd" id="l83">
<td class="vc_file_line_number"><a href="#l83">83</a></td>

<td class="vc_file_line_text">}</td>
</tr>




<tr class="vc_row_odd" id="l84">
<td class="vc_file_line_number"><a href="#l84">84</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l85">
<td class="vc_file_line_number"><a href="#l85">85</a></td>

<td class="vc_file_line_text">void</td>
</tr>




<tr class="vc_row_odd" id="l86">
<td class="vc_file_line_number"><a href="#l86">86</a></td>

<td class="vc_file_line_text">uuid_enc_be(void *buf, const uuid_t *uuid)</td>
</tr>




<tr class="vc_row_odd" id="l87">
<td class="vc_file_line_number"><a href="#l87">87</a></td>

<td class="vc_file_line_text">{</td>
</tr>




<tr class="vc_row_odd" id="l88">
<td class="vc_file_line_number"><a href="#l88">88</a></td>

<td class="vc_file_line_text">        uint8_t *p = buf;</td>
</tr>




<tr class="vc_row_odd" id="l89">
<td class="vc_file_line_number"><a href="#l89">89</a></td>

<td class="vc_file_line_text">        int i;</td>
</tr>




<tr class="vc_row_odd" id="l90">
<td class="vc_file_line_number"><a href="#l90">90</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l91">
<td class="vc_file_line_number"><a href="#l91">91</a></td>

<td class="vc_file_line_text">        be32enc(p, uuid-&gt;time_low);</td>
</tr>




<tr class="vc_row_odd" id="l92">
<td class="vc_file_line_number"><a href="#l92">92</a></td>

<td class="vc_file_line_text">        be16enc(p + 4, uuid-&gt;time_mid);</td>
</tr>




<tr class="vc_row_odd" id="l93">
<td class="vc_file_line_number"><a href="#l93">93</a></td>

<td class="vc_file_line_text">        be16enc(p + 6, uuid-&gt;time_hi_and_version);</td>
</tr>




<tr class="vc_row_odd" id="l94">
<td class="vc_file_line_number"><a href="#l94">94</a></td>

<td class="vc_file_line_text">        p[8] = uuid-&gt;clock_seq_hi_and_reserved;</td>
</tr>




<tr class="vc_row_odd" id="l95">
<td class="vc_file_line_number"><a href="#l95">95</a></td>

<td class="vc_file_line_text">        p[9] = uuid-&gt;clock_seq_low;</td>
</tr>




<tr class="vc_row_odd" id="l96">
<td class="vc_file_line_number"><a href="#l96">96</a></td>

<td class="vc_file_line_text">        for (i = 0; i &lt; _UUID_NODE_LEN; i++)</td>
</tr>




<tr class="vc_row_odd" id="l97">
<td class="vc_file_line_number"><a href="#l97">97</a></td>

<td class="vc_file_line_text">                p[10 + i] = uuid-&gt;node[i];</td>
</tr>




<tr class="vc_row_odd" id="l98">
<td class="vc_file_line_number"><a href="#l98">98</a></td>

<td class="vc_file_line_text">}</td>
</tr>




<tr class="vc_row_odd" id="l99">
<td class="vc_file_line_number"><a href="#l99">99</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l100">
<td class="vc_file_line_number"><a href="#l100">100</a></td>

<td class="vc_file_line_text">void</td>
</tr>




<tr class="vc_row_odd" id="l101">
<td class="vc_file_line_number"><a href="#l101">101</a></td>

<td class="vc_file_line_text">uuid_dec_be(const void *buf, uuid_t *uuid)</td>
</tr>




<tr class="vc_row_odd" id="l102">
<td class="vc_file_line_number"><a href="#l102">102</a></td>

<td class="vc_file_line_text">{</td>
</tr>




<tr class="vc_row_odd" id="l103">
<td class="vc_file_line_number"><a href="#l103">103</a></td>

<td class="vc_file_line_text">        const uint8_t *p = buf;</td>
</tr>




<tr class="vc_row_odd" id="l104">
<td class="vc_file_line_number"><a href="#l104">104</a></td>

<td class="vc_file_line_text">        int i;</td>
</tr>




<tr class="vc_row_odd" id="l105">
<td class="vc_file_line_number"><a href="#l105">105</a></td>

<td class="vc_file_line_text"></td>
</tr>




<tr class="vc_row_odd" id="l106">
<td class="vc_file_line_number"><a href="#l106">106</a></td>

<td class="vc_file_line_text">        uuid-&gt;time_low = be32dec(p);</td>
</tr>




<tr class="vc_row_odd" id="l107">
<td class="vc_file_line_number"><a href="#l107">107</a></td>

<td class="vc_file_line_text">        uuid-&gt;time_mid = be16dec(p + 4);</td>
</tr>




<tr class="vc_row_odd" id="l108">
<td class="vc_file_line_number"><a href="#l108">108</a></td>

<td class="vc_file_line_text">        uuid-&gt;time_hi_and_version = be16dec(p + 6);</td>
</tr>




<tr class="vc_row_odd" id="l109">
<td class="vc_file_line_number"><a href="#l109">109</a></td>

<td class="vc_file_line_text">        uuid-&gt;clock_seq_hi_and_reserved = p[8];</td>
</tr>




<tr class="vc_row_odd" id="l110">
<td class="vc_file_line_number"><a href="#l110">110</a></td>

<td class="vc_file_line_text">        uuid-&gt;clock_seq_low = p[9];</td>
</tr>




<tr class="vc_row_odd" id="l111">
<td class="vc_file_line_number"><a href="#l111">111</a></td>

<td class="vc_file_line_text">        for (i = 0; i &lt; _UUID_NODE_LEN; i++)</td>
</tr>




<tr class="vc_row_odd" id="l112">
<td class="vc_file_line_number"><a href="#l112">112</a></td>

<td class="vc_file_line_text">                uuid-&gt;node[i] = p[10 + i];</td>
</tr>




<tr class="vc_row_odd" id="l113">
<td class="vc_file_line_number"><a href="#l113">113</a></td>

<td class="vc_file_line_text">}</td>
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

