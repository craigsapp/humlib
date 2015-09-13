---
layout: default
breadcrumbs: [
                ['/',             'home'],
                ['/doc/class',    'classes'],
                ['/doc/snippet',  'snippets'],
                ['/doc/example',  'examples'],
                ['/doc/topic',    'topics'],
                ['/doc/tutorial', 'tutorials'],
                ['/doc/ref',      'reference']
        ]
title: Topics
---

{% include topics/topics.html %}

<p>
[<span style="cursor:pointer; color:#1e6bb8;" onclick="openAllDetails()">Open all</span>]
[<span style="cursor:pointer; color:#1e6bb8;" onclick="closeAllDetails()">Close all</span>]
</p>


<details class="topic-cvs">
<summary>
CVS Import/Export
</summary>
{% include topics/cvs.html %}
</details>

<details class="topic-xml">
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

<details class="topic-layout">
<summary>
Layout parameters
</summary>
{% include topics/layout.html %}
</details>

<details class="topic-rhythms">
<summary>
Rational rhythms
</summary>
{% include topics/rhythms.html %}
</details>

<details class="topic-strands">
<summary>
Strands
</summary>
{% include topics/strand.html %}
</details>

