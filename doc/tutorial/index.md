---
layout: default
breadcrumbs: [
                ['/',             'home'],
                ['/doc',          'documentation'],
                ['/doc/class',    'classes'],
                ['/doc/example',  'examples'],
                ['/doc/snippet',  'snippets'],
                ['/doc/topic',    'topics']
        ]
title: Tutorials
---

<style>

h2 {
	font-size: 115%;
}

</style>

{% include tutorials/tutorials.html %}

<p>
[<span style="cursor:pointer; color:#1e6bb8;" onclick="openAllTutorials()">Open all</span>]
[<span style="cursor:pointer; color:#1e6bb8;" onclick="closeAllTutorials()">Close all</span>]
</p>


<details class="tutorial-start">
<summary>
Getting started.
</summary>
{% include tutorials/start.md %}
</details>

<details class="tutorial-read">
<summary>
Reading Humdrum data.
</summary>
{% include tutorials/read.md %}
</details>


