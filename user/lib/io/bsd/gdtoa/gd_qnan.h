<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>gd_qnan.h</title>
<style type="text/css">
.enscript-comment { font-style: italic; color: rgb(178,34,34); }
.enscript-function-name { font-weight: bold; color: rgb(0,0,255); }
.enscript-variable-name { font-weight: bold; color: rgb(184,134,11); }
.enscript-keyword { font-weight: bold; color: rgb(160,32,240); }
.enscript-reference { font-weight: bold; color: rgb(95,158,160); }
.enscript-string { font-weight: bold; color: rgb(188,143,143); }
.enscript-builtin { font-weight: bold; color: rgb(218,112,214); }
.enscript-type { font-weight: bold; color: rgb(34,139,34); }
.enscript-highlight { text-decoration: underline; color: 0; }
</style>
</head>
<body id="top">
<h1 style="margin:8px;" id="f1">gd_qnan.h&nbsp;&nbsp;&nbsp;<span style="font-weight: normal; font-size: 0.5em;">[<a href="?txt">plain text</a>]</span></h1>
<hr/>
<div></div>
<pre>
#<span class="enscript-reference">if</span> <span class="enscript-reference">defined</span>(<span class="enscript-variable-name">__ppc__</span>) || <span class="enscript-reference">defined</span>(<span class="enscript-variable-name">__ppc64__</span>)

#<span class="enscript-reference">define</span> <span class="enscript-variable-name">f_QNAN</span> 0x7fc00000
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">d_QNAN0</span> 0x7ff80000
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">d_QNAN1</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN0</span> 0x7ff80000
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN1</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN2</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN3</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN0</span> 0x7ff8
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN1</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN2</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN3</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN4</span> 0x0

#<span class="enscript-reference">elif</span> <span class="enscript-reference">defined</span>(<span class="enscript-variable-name">__i386__</span>) || <span class="enscript-reference">defined</span>(<span class="enscript-variable-name">__x86_64__</span>) || <span class="enscript-reference">defined</span>(<span class="enscript-variable-name">__arm__</span>) 

#<span class="enscript-reference">define</span> <span class="enscript-variable-name">f_QNAN</span> 0x7fc00000
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">d_QNAN0</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">d_QNAN1</span> 0x7ff80000
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN0</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN1</span> 0xc0000000
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN2</span> 0x7fff
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ld_QNAN3</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN0</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN1</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN2</span> 0x0
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN3</span> 0xc000
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">ldus_QNAN4</span> 0x7fff

#<span class="enscript-reference">else</span>
#<span class="enscript-reference">error</span> <span class="enscript-variable-name">unknown</span> <span class="enscript-variable-name">architecture</span>
#<span class="enscript-reference">endif</span>
</pre>
<hr />
</body></html>