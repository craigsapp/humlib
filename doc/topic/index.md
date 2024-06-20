---
title: Topics
layout: default
vim: ft=markdown:nowrap
breadcrumbs: [
                ['/',             'home'],
                ['/doc/class',    'classes'],
                ['/doc/snippet',  'snippets'],
                ['/doc/example',  'examples'],
                ['/doc/topic',    'topics'],
                ['/doc/tutorial', 'tutorials'],
                ['/doc/ref',      'reference']
            ]
---

{% include_relative scripts-local.html %}
{% include_relative styles-local.html %}

<p>
[<span style="cursor:pointer; color:#1e6bb8;" onclick="openAllDetails()">Open all</span>]
[<span style="cursor:pointer; color:#1e6bb8;" onclick="closeAllDetails()">Close all</span>]
</p>


<details class="topic-csv" markdown="1">
<summary> CSV Import/Export </summary>
{% include_relative csv.md %}
</details>

<details class="topic-xml" markdown="1">
<summary> XML Export </summary>
{% include_relative xml.md %}
</details>

<details class="topic-parameters" markdown="1">
<summary> Parameters </summary>
{% include_relative parameters.md %}
</details>

<details class="topic-layout" markdown="1">
<summary> Layout parameters </summary>
{% include_relative layout.md %}
</details>

<details class="topic-rhythms" markdown="1">
<summary> Rational rhythms </summary>
{% include_relative rhythms.md %}
</details>

<details class="topic-strands" markdown="1">
<summary> Strands </summary>
{% include_relative strands.md %}
</details>

