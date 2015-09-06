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

<a name="cvs"> </a>
<details class="topic-cvs">
<summary>
CVS Import/Export
</summary>
{% include topics/cvs.html %}
</details>

<a name="xml"> </a>
<details class="topic-xml">
<summary>
XML Export
</summary>
{% include topics/xml.html %}
</details>

<a name="parameter"> </a>
<a name="parameters"> </a>
<details class="topic-parameters">
<summary>
Parameters
</summary>
{% include topics/parameters.html %}
</details>

<a name="rhythm"> </a>
<a name="rhythms"> </a>
<details class="topic-rhythms">
<summary>
Rational rhythms
</summary>
{% include topics/rhythms.html %}
</details>

<a name="strand"> </a>
<a name="strands"> </a>
<details class="topic-parameters">
<summary>
Strands
</summary>
{% include topics/strand.html %}
</details>

