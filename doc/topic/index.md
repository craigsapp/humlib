---
layout: default
breadcrumbs: [
                ['/', 'home'],
                ['/doc', 'documentation'],
                ['/doc/class',    'classes'],
                ['/doc/example',  'examples'],
                ['/doc/snippet',  'snippets'],
                ['/doc/topic', 'topics']
        ]
title: Topics
---

[<span style="cursor:pointer; color:#1e6bb8;" onclick="openAllTopics()">Open all</span>]
[<span style="cursor:pointer; color:#1e6bb8;" onclick="closeAllTopics()">Close all</span>]

{% include topics/topics.html %}

<details class="topic-cvs">
<summary>
CVS Import/Export
</summary>
{% include topics/cvs.html %}
</details>

<details class="topic-cvs">
<summary>
XML Export
</summary>
{% include topics/xml.html %}
</details>

<details class="topic-parameters">
<summary>
Parameters
</summary>
{% include topics/parameters.html %}
</details>

<details class="topic-rhythms">
<summary>
Rational rhythms
</summary>
{% include topics/rhythms.html %}
</details>

<details class="topic-parameters">
<summary>
Strands
</summary>
{% include topics/strand.html %}
</details>

