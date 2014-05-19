




<!DOCTYPE html>
<html class="   ">
  <head prefix="og: http://ogp.me/ns# fb: http://ogp.me/ns/fb# object: http://ogp.me/ns/object# article: http://ogp.me/ns/article# profile: http://ogp.me/ns/profile#">
    <meta charset='utf-8'>
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    
    
    <title>illumos-gate/usr/src/lib/libbc/libc/gen/sys5/sgetl.c at master · illumos/illumos-gate · GitHub</title>
    <link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="GitHub" />
    <link rel="fluid-icon" href="https://github.com/fluidicon.png" title="GitHub" />
    <link rel="apple-touch-icon" sizes="57x57" href="/apple-touch-icon-114.png" />
    <link rel="apple-touch-icon" sizes="114x114" href="/apple-touch-icon-114.png" />
    <link rel="apple-touch-icon" sizes="72x72" href="/apple-touch-icon-144.png" />
    <link rel="apple-touch-icon" sizes="144x144" href="/apple-touch-icon-144.png" />
    <meta property="fb:app_id" content="1401488693436528"/>

      <meta content="@github" name="twitter:site" /><meta content="summary" name="twitter:card" /><meta content="illumos/illumos-gate" name="twitter:title" /><meta content="illumos-gate - Community developed and maintained version of the OS/Net consolidation" name="twitter:description" /><meta content="https://avatars0.githubusercontent.com/u/383551?s=400" name="twitter:image:src" />
<meta content="GitHub" property="og:site_name" /><meta content="object" property="og:type" /><meta content="https://avatars0.githubusercontent.com/u/383551?s=400" property="og:image" /><meta content="illumos/illumos-gate" property="og:title" /><meta content="https://github.com/illumos/illumos-gate" property="og:url" /><meta content="illumos-gate - Community developed and maintained version of the OS/Net consolidation" property="og:description" />

    <link rel="assets" href="https://assets-cdn.github.com/">
    <link rel="conduit-xhr" href="https://ghconduit.com:25035/">
    <link rel="xhr-socket" href="/_sockets" />

    <meta name="msapplication-TileImage" content="/windows-tile.png" />
    <meta name="msapplication-TileColor" content="#ffffff" />
    <meta name="selected-link" value="repo_source" data-pjax-transient />
    <meta content="collector.githubapp.com" name="octolytics-host" /><meta content="collector-cdn.github.com" name="octolytics-script-host" /><meta content="github" name="octolytics-app-id" /><meta content="021CC16B:3040:ADFDBF6:537A5AE7" name="octolytics-dimension-request_id" />
    

    
    
    <link rel="icon" type="image/x-icon" href="https://assets-cdn.github.com/favicon.ico" />

    <meta content="authenticity_token" name="csrf-param" />
<meta content="VBTp3a//gQruZdh7czsAIFU/bEr3+b/Ly2weAIM9WXnXm44OZsqkKCsDv3NyFyacTHbkRuXMQkLSrjXGRzMuvQ==" name="csrf-token" />

    <link href="https://assets-cdn.github.com/assets/github-6ea0c0e56db1fb59238068401b012808e41ae7c5.css" media="all" rel="stylesheet" type="text/css" />
    <link href="https://assets-cdn.github.com/assets/github2-a49192fef33ef6e2053a634597427056eb62d634.css" media="all" rel="stylesheet" type="text/css" />
    


    <meta http-equiv="x-pjax-version" content="884c875af12d4668e58c079de207bf79">

      
  <meta name="description" content="illumos-gate - Community developed and maintained version of the OS/Net consolidation" />

  <meta content="383551" name="octolytics-dimension-user_id" /><meta content="illumos" name="octolytics-dimension-user_login" /><meta content="894162" name="octolytics-dimension-repository_id" /><meta content="illumos/illumos-gate" name="octolytics-dimension-repository_nwo" /><meta content="true" name="octolytics-dimension-repository_public" /><meta content="false" name="octolytics-dimension-repository_is_fork" /><meta content="894162" name="octolytics-dimension-repository_network_root_id" /><meta content="illumos/illumos-gate" name="octolytics-dimension-repository_network_root_nwo" />
  <link href="https://github.com/illumos/illumos-gate/commits/master.atom" rel="alternate" title="Recent Commits to illumos-gate:master" type="application/atom+xml" />

  </head>


  <body class="logged_out  env-production  vis-public page-blob">
    <a href="#start-of-content" tabindex="1" class="accessibility-aid js-skip-to-content">Skip to content</a>
    <div class="wrapper">
      
      
      
      


      
      <div class="header header-logged-out">
  <div class="container clearfix">

    <a class="header-logo-wordmark" href="https://github.com/">
      <span class="mega-octicon octicon-logo-github"></span>
    </a>

    <div class="header-actions">
        <a class="button primary" href="/join">Sign up</a>
      <a class="button signin" href="/login?return_to=%2Fillumos%2Fillumos-gate%2Fblob%2Fmaster%2Fusr%2Fsrc%2Flib%2Flibbc%2Flibc%2Fgen%2Fsys5%2Fsgetl.c">Sign in</a>
    </div>

    <div class="command-bar js-command-bar  in-repository">

      <ul class="top-nav">
          <li class="explore"><a href="/explore">Explore</a></li>
        <li class="features"><a href="/features">Features</a></li>
          <li class="enterprise"><a href="https://enterprise.github.com/">Enterprise</a></li>
          <li class="blog"><a href="/blog">Blog</a></li>
      </ul>
        <form accept-charset="UTF-8" action="/search" class="command-bar-form" id="top_search_form" method="get">

<div class="commandbar">
  <span class="message"></span>
  <input type="text" data-hotkey="s, /" name="q" id="js-command-bar-field" placeholder="Search or type a command" tabindex="1" autocapitalize="off"
    
    
      data-repo="illumos/illumos-gate"
      data-branch="master"
      data-sha="3fd8815f466051e2274f7d101fd52885606a4366"
  >
  <div class="display hidden"></div>
</div>

    <input type="hidden" name="nwo" value="illumos/illumos-gate" />

    <div class="select-menu js-menu-container js-select-menu search-context-select-menu">
      <span class="minibutton select-menu-button js-menu-target" role="button" aria-haspopup="true">
        <span class="js-select-button">This repository</span>
      </span>

      <div class="select-menu-modal-holder js-menu-content js-navigation-container" aria-hidden="true">
        <div class="select-menu-modal">

          <div class="select-menu-item js-navigation-item js-this-repository-navigation-item selected">
            <span class="select-menu-item-icon octicon octicon-check"></span>
            <input type="radio" class="js-search-this-repository" name="search_target" value="repository" checked="checked" />
            <div class="select-menu-item-text js-select-button-text">This repository</div>
          </div> <!-- /.select-menu-item -->

          <div class="select-menu-item js-navigation-item js-all-repositories-navigation-item">
            <span class="select-menu-item-icon octicon octicon-check"></span>
            <input type="radio" name="search_target" value="global" />
            <div class="select-menu-item-text js-select-button-text">All repositories</div>
          </div> <!-- /.select-menu-item -->

        </div>
      </div>
    </div>

  <span class="help tooltipped tooltipped-s" aria-label="Show command bar help">
    <span class="octicon octicon-question"></span>
  </span>


  <input type="hidden" name="ref" value="cmdform">

</form>
    </div>

  </div>
</div>



      <div id="start-of-content" class="accessibility-aid"></div>
          <div class="site" itemscope itemtype="http://schema.org/WebPage">
    <div id="js-flash-container">
      
    </div>
    <div class="pagehead repohead instapaper_ignore readability-menu">
      <div class="container">
        

<ul class="pagehead-actions">


  <li>
    <a href="/login?return_to=%2Fillumos%2Fillumos-gate"
    class="minibutton with-count star-button tooltipped tooltipped-n"
    aria-label="You must be signed in to star a repository" rel="nofollow">
    <span class="octicon octicon-star"></span>Star
  </a>

    <a class="social-count js-social-count" href="/illumos/illumos-gate/stargazers">
      220
    </a>

  </li>

    <li>
      <a href="/login?return_to=%2Fillumos%2Fillumos-gate"
        class="minibutton with-count js-toggler-target fork-button tooltipped tooltipped-n"
        aria-label="You must be signed in to fork a repository" rel="nofollow">
        <span class="octicon octicon-git-branch"></span>Fork
      </a>
      <a href="/illumos/illumos-gate/network" class="social-count">
        163
      </a>
    </li>
</ul>

        <h1 itemscope itemtype="http://data-vocabulary.org/Breadcrumb" class="entry-title public">
          <span class="repo-label"><span>public</span></span>
          <span class="mega-octicon octicon-repo"></span>
          <span class="author"><a href="/illumos" class="url fn" itemprop="url" rel="author"><span itemprop="title">illumos</span></a></span><!--
       --><span class="path-divider">/</span><!--
       --><strong><a href="/illumos/illumos-gate" class="js-current-repository js-repo-home-link">illumos-gate</a></strong>

          <span class="page-context-loader">
            <img alt="" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
          </span>

        </h1>
      </div><!-- /.container -->
    </div><!-- /.repohead -->

    <div class="container">
      <div class="repository-with-sidebar repo-container new-discussion-timeline js-new-discussion-timeline  ">
        <div class="repository-sidebar clearfix">
            

<div class="sunken-menu vertical-right repo-nav js-repo-nav js-repository-container-pjax js-octicon-loaders">
  <div class="sunken-menu-contents">
    <ul class="sunken-menu-group">
      <li class="tooltipped tooltipped-w" aria-label="Code">
        <a href="/illumos/illumos-gate" aria-label="Code" class="selected js-selected-navigation-item sunken-menu-item" data-hotkey="g c" data-pjax="true" data-selected-links="repo_source repo_downloads repo_commits repo_releases repo_tags repo_branches /illumos/illumos-gate">
          <span class="octicon octicon-code"></span> <span class="full-word">Code</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>


      <li class="tooltipped tooltipped-w" aria-label="Pull Requests">
        <a href="/illumos/illumos-gate/pulls" aria-label="Pull Requests" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-hotkey="g p" data-selected-links="repo_pulls /illumos/illumos-gate/pulls">
            <span class="octicon octicon-git-pull-request"></span> <span class="full-word">Pull Requests</span>
            <span class='counter'>0</span>
            <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>


    </ul>
    <div class="sunken-menu-separator"></div>
    <ul class="sunken-menu-group">

      <li class="tooltipped tooltipped-w" aria-label="Pulse">
        <a href="/illumos/illumos-gate/pulse" aria-label="Pulse" class="js-selected-navigation-item sunken-menu-item" data-pjax="true" data-selected-links="pulse /illumos/illumos-gate/pulse">
          <span class="octicon octicon-pulse"></span> <span class="full-word">Pulse</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

      <li class="tooltipped tooltipped-w" aria-label="Graphs">
        <a href="/illumos/illumos-gate/graphs" aria-label="Graphs" class="js-selected-navigation-item sunken-menu-item" data-pjax="true" data-selected-links="repo_graphs repo_contributors /illumos/illumos-gate/graphs">
          <span class="octicon octicon-graph"></span> <span class="full-word">Graphs</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

      <li class="tooltipped tooltipped-w" aria-label="Network">
        <a href="/illumos/illumos-gate/network" aria-label="Network" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-selected-links="repo_network /illumos/illumos-gate/network">
          <span class="octicon octicon-git-branch"></span> <span class="full-word">Network</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>
    </ul>


  </div>
</div>

              <div class="only-with-full-nav">
                

  

<div class="clone-url open"
  data-protocol-type="http"
  data-url="/users/set_protocol?protocol_selector=http&amp;protocol_type=clone">
  <h3><strong>HTTPS</strong> clone URL</h3>
  <div class="clone-url-box">
    <input type="text" class="clone js-url-field"
           value="https://github.com/illumos/illumos-gate.git" readonly="readonly">
    <span class="url-box-clippy">
    <button aria-label="copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="https://github.com/illumos/illumos-gate.git" data-copied-hint="copied!" type="button"><span class="octicon octicon-clippy"></span></button>
    </span>
  </div>
</div>

  

<div class="clone-url "
  data-protocol-type="subversion"
  data-url="/users/set_protocol?protocol_selector=subversion&amp;protocol_type=clone">
  <h3><strong>Subversion</strong> checkout URL</h3>
  <div class="clone-url-box">
    <input type="text" class="clone js-url-field"
           value="https://github.com/illumos/illumos-gate" readonly="readonly">
    <span class="url-box-clippy">
    <button aria-label="copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="https://github.com/illumos/illumos-gate" data-copied-hint="copied!" type="button"><span class="octicon octicon-clippy"></span></button>
    </span>
  </div>
</div>


<p class="clone-options">You can clone with
      <a href="#" class="js-clone-selector" data-protocol="http">HTTPS</a>
      or <a href="#" class="js-clone-selector" data-protocol="subversion">Subversion</a>.
  <span class="help tooltipped tooltipped-n" aria-label="Get help on which URL is right for you.">
    <a href="https://help.github.com/articles/which-remote-url-should-i-use">
    <span class="octicon octicon-question"></span>
    </a>
  </span>
</p>



                <a href="/illumos/illumos-gate/archive/master.zip"
                   class="minibutton sidebar-button"
                   aria-label="Download illumos/illumos-gate as a zip file"
                   title="Download illumos/illumos-gate as a zip file"
                   rel="nofollow">
                  <span class="octicon octicon-cloud-download"></span>
                  Download ZIP
                </a>
              </div>
        </div><!-- /.repository-sidebar -->

        <div id="js-repo-pjax-container" class="repository-content context-loader-container" data-pjax-container>
          


<a href="/illumos/illumos-gate/blob/a65cd518c5d0f30c53594a7022eb0f7d04c98cef/usr/src/lib/libbc/libc/gen/sys5/sgetl.c" class="hidden js-permalink-shortcut" data-hotkey="y">Permalink</a>

<!-- blob contrib key: blob_contributors:v21:5b152f4bd461ed638f5dc497769e8471 -->

<p title="This is a placeholder element" class="js-history-link-replace hidden"></p>

<a href="/illumos/illumos-gate/find/master" data-pjax data-hotkey="t" class="js-show-file-finder" style="display:none">Show File Finder</a>

<div class="file-navigation">
  

<div class="select-menu js-menu-container js-select-menu" >
  <span class="minibutton select-menu-button js-menu-target" data-hotkey="w"
    data-master-branch="master"
    data-ref="master"
    role="button" aria-label="Switch branches or tags" tabindex="0" aria-haspopup="true">
    <span class="octicon octicon-git-branch"></span>
    <i>branch:</i>
    <span class="js-select-button">master</span>
  </span>

  <div class="select-menu-modal-holder js-menu-content js-navigation-container" data-pjax aria-hidden="true">

    <div class="select-menu-modal">
      <div class="select-menu-header">
        <span class="select-menu-title">Switch branches/tags</span>
        <span class="octicon octicon-remove-close js-menu-close"></span>
      </div> <!-- /.select-menu-header -->

      <div class="select-menu-filters">
        <div class="select-menu-text-filter">
          <input type="text" aria-label="Filter branches/tags" id="context-commitish-filter-field" class="js-filterable-field js-navigation-enable" placeholder="Filter branches/tags">
        </div>
        <div class="select-menu-tabs">
          <ul>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="branches" class="js-select-menu-tab">Branches</a>
            </li>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="tags" class="js-select-menu-tab">Tags</a>
            </li>
          </ul>
        </div><!-- /.select-menu-tabs -->
      </div><!-- /.select-menu-filters -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="branches">

        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


            <div class="select-menu-item js-navigation-item selected">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/illumos/illumos-gate/blob/master/usr/src/lib/libbc/libc/gen/sys5/sgetl.c"
                 data-name="master"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text js-select-button-text css-truncate-target"
                 title="master">master</a>
            </div> <!-- /.select-menu-item -->
        </div>

          <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="tags">
        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


        </div>

        <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

    </div> <!-- /.select-menu-modal -->
  </div> <!-- /.select-menu-modal-holder -->
</div> <!-- /.select-menu -->

  <div class="breadcrumb">
    <span class='repo-root js-repo-root'><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">illumos-gate</span></a></span></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate/tree/master/usr" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">usr</span></a></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate/tree/master/usr/src" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">src</span></a></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate/tree/master/usr/src/lib" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">lib</span></a></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate/tree/master/usr/src/lib/libbc" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">libbc</span></a></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate/tree/master/usr/src/lib/libbc/libc" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">libc</span></a></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate/tree/master/usr/src/lib/libbc/libc/gen" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">gen</span></a></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/illumos/illumos-gate/tree/master/usr/src/lib/libbc/libc/gen/sys5" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">sys5</span></a></span><span class="separator"> / </span><strong class="final-path">sgetl.c</strong> <button aria-label="copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="usr/src/lib/libbc/libc/gen/sys5/sgetl.c" data-copied-hint="copied!" type="button"><span class="octicon octicon-clippy"></span></button>
  </div>
</div>


  <div class="commit file-history-tease">
      <img class="main-avatar" height="24" src="https://1.gravatar.com/avatar/b5cf4bf7b06f79b279e0967f8a253183?d=https%3A%2F%2Fgithub.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png&amp;r=x&amp;s=140" width="24" />
      <span class="author"><span>stevel@tonic-gate</span></span>
      <time datetime="2005-06-14T00:00:00-07:00" is="relative-time" title-format="%Y-%m-%d %H:%M:%S %z" title="2005-06-14 00:00:00 -0700">June 14, 2005</time>
      <div class="commit-title">
          <a href="/illumos/illumos-gate/commit/7c478bd95313f5f23a4c958a745db2134aa03244" class="message" data-pjax="true" title="OpenSolaris Launch">OpenSolaris Launch</a>
      </div>

    <div class="participation">
      <p class="quickstat"><a href="#blob_contributors_box" rel="facebox"><strong>0</strong>  contributors</a></p>
      
    </div>
    <div id="blob_contributors_box" style="display:none">
      <h2 class="facebox-header">Users who have contributed to this file</h2>
      <ul class="facebox-user-list">
      </ul>
    </div>
  </div>

<div class="file-box">
  <div class="file">
    <div class="meta clearfix">
      <div class="info file-name">
        <span class="icon"><b class="octicon octicon-file-text"></b></span>
        <span class="mode" title="File Mode">file</span>
        <span class="meta-divider"></span>
          <span>50 lines (44 sloc)</span>
          <span class="meta-divider"></span>
        <span>1.495 kb</span>
      </div>
      <div class="actions">
        <div class="button-group">
              <a class="minibutton disabled tooltipped tooltipped-w" href="#"
                 aria-label="You must be signed in to make or propose changes">Edit</a>
          <a href="/illumos/illumos-gate/raw/master/usr/src/lib/libbc/libc/gen/sys5/sgetl.c" class="button minibutton " id="raw-url">Raw</a>
            <a href="/illumos/illumos-gate/blame/master/usr/src/lib/libbc/libc/gen/sys5/sgetl.c" class="button minibutton js-update-url-with-hash">Blame</a>
          <a href="/illumos/illumos-gate/commits/master/usr/src/lib/libbc/libc/gen/sys5/sgetl.c" class="button minibutton " rel="nofollow">History</a>
        </div><!-- /.button-group -->
          <a class="minibutton danger disabled empty-icon tooltipped tooltipped-w" href="#"
             aria-label="You must be signed in to make or propose changes">
          Delete
        </a>
      </div><!-- /.actions -->
    </div>
        <div class="blob-wrapper data type-c js-blob-data">
        <table class="file-code file-diff tab-size-8">
          <tr class="file-code-line">
            <td class="blob-line-nums">
              <span id="L1" rel="#L1">1</span>
<span id="L2" rel="#L2">2</span>
<span id="L3" rel="#L3">3</span>
<span id="L4" rel="#L4">4</span>
<span id="L5" rel="#L5">5</span>
<span id="L6" rel="#L6">6</span>
<span id="L7" rel="#L7">7</span>
<span id="L8" rel="#L8">8</span>
<span id="L9" rel="#L9">9</span>
<span id="L10" rel="#L10">10</span>
<span id="L11" rel="#L11">11</span>
<span id="L12" rel="#L12">12</span>
<span id="L13" rel="#L13">13</span>
<span id="L14" rel="#L14">14</span>
<span id="L15" rel="#L15">15</span>
<span id="L16" rel="#L16">16</span>
<span id="L17" rel="#L17">17</span>
<span id="L18" rel="#L18">18</span>
<span id="L19" rel="#L19">19</span>
<span id="L20" rel="#L20">20</span>
<span id="L21" rel="#L21">21</span>
<span id="L22" rel="#L22">22</span>
<span id="L23" rel="#L23">23</span>
<span id="L24" rel="#L24">24</span>
<span id="L25" rel="#L25">25</span>
<span id="L26" rel="#L26">26</span>
<span id="L27" rel="#L27">27</span>
<span id="L28" rel="#L28">28</span>
<span id="L29" rel="#L29">29</span>
<span id="L30" rel="#L30">30</span>
<span id="L31" rel="#L31">31</span>
<span id="L32" rel="#L32">32</span>
<span id="L33" rel="#L33">33</span>
<span id="L34" rel="#L34">34</span>
<span id="L35" rel="#L35">35</span>
<span id="L36" rel="#L36">36</span>
<span id="L37" rel="#L37">37</span>
<span id="L38" rel="#L38">38</span>
<span id="L39" rel="#L39">39</span>
<span id="L40" rel="#L40">40</span>
<span id="L41" rel="#L41">41</span>
<span id="L42" rel="#L42">42</span>
<span id="L43" rel="#L43">43</span>
<span id="L44" rel="#L44">44</span>
<span id="L45" rel="#L45">45</span>
<span id="L46" rel="#L46">46</span>
<span id="L47" rel="#L47">47</span>
<span id="L48" rel="#L48">48</span>
<span id="L49" rel="#L49">49</span>

            </td>
            <td class="blob-line-code"><div class="code-body highlight"><pre><div class='line' id='LC1'><span class="cm">/*</span></div><div class='line' id='LC2'><span class="cm"> * CDDL HEADER START</span></div><div class='line' id='LC3'><span class="cm"> *</span></div><div class='line' id='LC4'><span class="cm"> * The contents of this file are subject to the terms of the</span></div><div class='line' id='LC5'><span class="cm"> * Common Development and Distribution License, Version 1.0 only</span></div><div class='line' id='LC6'><span class="cm"> * (the &quot;License&quot;).  You may not use this file except in compliance</span></div><div class='line' id='LC7'><span class="cm"> * with the License.</span></div><div class='line' id='LC8'><span class="cm"> *</span></div><div class='line' id='LC9'><span class="cm"> * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE</span></div><div class='line' id='LC10'><span class="cm"> * or http://www.opensolaris.org/os/licensing.</span></div><div class='line' id='LC11'><span class="cm"> * See the License for the specific language governing permissions</span></div><div class='line' id='LC12'><span class="cm"> * and limitations under the License.</span></div><div class='line' id='LC13'><span class="cm"> *</span></div><div class='line' id='LC14'><span class="cm"> * When distributing Covered Code, include this CDDL HEADER in each</span></div><div class='line' id='LC15'><span class="cm"> * file and include the License file at usr/src/OPENSOLARIS.LICENSE.</span></div><div class='line' id='LC16'><span class="cm"> * If applicable, add the following below this CDDL HEADER, with the</span></div><div class='line' id='LC17'><span class="cm"> * fields enclosed by brackets &quot;[]&quot; replaced with your own identifying</span></div><div class='line' id='LC18'><span class="cm"> * information: Portions Copyright [yyyy] [name of copyright owner]</span></div><div class='line' id='LC19'><span class="cm"> *</span></div><div class='line' id='LC20'><span class="cm"> * CDDL HEADER END</span></div><div class='line' id='LC21'><span class="cm"> */</span></div><div class='line' id='LC22'><span class="cm">/*      Copyright (c) 1984 AT&amp;T */</span></div><div class='line' id='LC23'><span class="cm">/*        All Rights Reserved   */</span></div><div class='line' id='LC24'><br/></div><div class='line' id='LC25'><span class="cp">#pragma ident	&quot;%Z%%M%	%I%	%E% SMI&quot;</span></div><div class='line' id='LC26'><span class="cm">/*</span></div><div class='line' id='LC27'><span class="cm"> * Provide machine independent transfer of longs.</span></div><div class='line' id='LC28'><span class="cm"> */</span></div><div class='line' id='LC29'><br/></div><div class='line' id='LC30'><span class="cm">/*</span></div><div class='line' id='LC31'><span class="cm"> * The intent here is to provide a means to make the value of</span></div><div class='line' id='LC32'><span class="cm"> * bytes in an io-buffer correspond to the value of a long</span></div><div class='line' id='LC33'><span class="cm"> * in the memory while doing the io a `long&#39; at a time.</span></div><div class='line' id='LC34'><span class="cm"> * Files written and read in this way are machine-independent.</span></div><div class='line' id='LC35'><span class="cm"> *</span></div><div class='line' id='LC36'><span class="cm"> */</span></div><div class='line' id='LC37'><span class="cp">#include &lt;values.h&gt;</span></div><div class='line' id='LC38'><br/></div><div class='line' id='LC39'><span class="kt">long</span>  <span class="n">sgetl</span><span class="p">(</span><span class="n">buffer</span><span class="p">)</span></div><div class='line' id='LC40'><span class="k">register</span> <span class="kt">char</span> <span class="o">*</span><span class="n">buffer</span><span class="p">;</span></div><div class='line' id='LC41'><span class="p">{</span></div><div class='line' id='LC42'>	 <span class="k">register</span> <span class="kt">long</span> <span class="n">l</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span></div><div class='line' id='LC43'>	 <span class="k">register</span> <span class="kt">int</span> <span class="n">i</span> <span class="o">=</span> <span class="n">BITSPERBYTE</span> <span class="o">*</span> <span class="k">sizeof</span><span class="p">(</span><span class="kt">long</span><span class="p">);</span></div><div class='line' id='LC44'><br/></div><div class='line' id='LC45'>	 <span class="k">while</span> <span class="p">((</span><span class="n">i</span> <span class="o">-=</span> <span class="n">BITSPERBYTE</span><span class="p">)</span> <span class="o">&gt;=</span> <span class="mi">0</span><span class="p">)</span></div><div class='line' id='LC46'>			 <span class="n">l</span> <span class="o">|=</span> <span class="p">(</span><span class="kt">short</span><span class="p">)</span> <span class="p">((</span><span class="kt">unsigned</span> <span class="kt">char</span><span class="p">)</span> <span class="o">*</span><span class="n">buffer</span><span class="o">++</span><span class="p">)</span> <span class="o">&lt;&lt;</span> <span class="n">i</span><span class="p">;</span></div><div class='line' id='LC47'>	 <span class="k">return</span>  <span class="n">l</span><span class="p">;</span></div><div class='line' id='LC48'>&nbsp;<span class="p">}</span></div><div class='line' id='LC49'><br/></div></pre></div></td>
          </tr>
        </table>
  </div>

  </div>
</div>

<a href="#jump-to-line" rel="facebox[.linejump]" data-hotkey="l" class="js-jump-to-line" style="display:none">Jump to Line</a>
<div id="jump-to-line" style="display:none">
  <form accept-charset="UTF-8" class="js-jump-to-line-form">
    <input class="linejump-input js-jump-to-line-field" type="text" placeholder="Jump to line&hellip;" autofocus>
    <button type="submit" class="button">Go</button>
  </form>
</div>

        </div>

      </div><!-- /.repo-container -->
      <div class="modal-backdrop"></div>
    </div><!-- /.container -->
  </div><!-- /.site -->


    </div><!-- /.wrapper -->

      <div class="container">
  <div class="site-footer">
    <ul class="site-footer-links right">
      <li><a href="https://status.github.com/">Status</a></li>
      <li><a href="http://developer.github.com">API</a></li>
      <li><a href="http://training.github.com">Training</a></li>
      <li><a href="http://shop.github.com">Shop</a></li>
      <li><a href="/blog">Blog</a></li>
      <li><a href="/about">About</a></li>

    </ul>

    <a href="/">
      <span class="mega-octicon octicon-mark-github" title="GitHub"></span>
    </a>

    <ul class="site-footer-links">
      <li>&copy; 2014 <span title="0.03244s from github-fe124-cp1-prd.iad.github.net">GitHub</span>, Inc.</li>
        <li><a href="/site/terms">Terms</a></li>
        <li><a href="/site/privacy">Privacy</a></li>
        <li><a href="/security">Security</a></li>
        <li><a href="/contact">Contact</a></li>
    </ul>
  </div><!-- /.site-footer -->
</div><!-- /.container -->


    <div class="fullscreen-overlay js-fullscreen-overlay" id="fullscreen_overlay">
  <div class="fullscreen-container js-fullscreen-container">
    <div class="textarea-wrap">
      <textarea name="fullscreen-contents" id="fullscreen-contents" class="fullscreen-contents js-fullscreen-contents" placeholder="" data-suggester="fullscreen_suggester"></textarea>
    </div>
  </div>
  <div class="fullscreen-sidebar">
    <a href="#" class="exit-fullscreen js-exit-fullscreen tooltipped tooltipped-w" aria-label="Exit Zen Mode">
      <span class="mega-octicon octicon-screen-normal"></span>
    </a>
    <a href="#" class="theme-switcher js-theme-switcher tooltipped tooltipped-w"
      aria-label="Switch themes">
      <span class="octicon octicon-color-mode"></span>
    </a>
  </div>
</div>



    <div id="ajax-error-message" class="flash flash-error">
      <span class="octicon octicon-alert"></span>
      <a href="#" class="octicon octicon-remove-close close js-ajax-error-dismiss"></a>
      Something went wrong with that request. Please try again.
    </div>


      <script crossorigin="anonymous" src="https://assets-cdn.github.com/assets/frameworks-9027ad6a9d00434697fea4d0143670c6fb7b2471.js" type="text/javascript"></script>
      <script async="async" crossorigin="anonymous" src="https://assets-cdn.github.com/assets/github-f8fc00b8934006933bc2391fd76f435ac85a7016.js" type="text/javascript"></script>
      
      
  </body>
</html>

