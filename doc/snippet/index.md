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
title: Code snippets
---

{% include snippets/snippets.html %}

Here are examples of how to access data in a Humdrum score using the
minHumdrum classes:

[<span style="cursor:pointer; color:#1e6bb8;" onclick="closeAllDetails()">Close all</span>]
[<span style="cursor:pointer; color:#1e6bb8;" onclick="openAllDetails()">Open all</span>]


<style>

body {
	counter-reset: details
}

summary::before {
	counter-increment: details;
	content: counter(details) " ";
}

</style>

<details open class="snippet-read">
<summary></summary>
{% include snippets/read.md %}
</details>

<details open class="snippet-token">
<summary></summary>
{% include snippets/token.md %}
</details>

<details open class="snippet-tokstring">
<summary></summary>
{% include snippets/tokstring.md %}
</details>

<details open class="snippet-tokdur">
<summary></summary>
{% include snippets/tokdur.md %}
</details>

<details open class="snippet-linedur">
<summary></summary>
{% include snippets/linedur.md %}
</details>

<details open class="snippet-filedur">
<summary></summary>
{% include snippets/filedur.md %}
</details>

<details open class="snippet-tpq">
<summary></summary>
{% include snippets/tpq.md %}
</details>

<details open class="snippet-tpqdur">
<summary></summary>
{% include snippets/tpqdur.md %}
</details>

<details open class="snippet-linecount">
<summary></summary>
{% include snippets/linecount.md %}
</details>

<details open class="snippet-fieldcount">
<summary></summary>
{% include snippets/fieldcount.md %}
</details>

<details open class="snippet-track">
<summary></summary>
{% include snippets/track.md %}
</details>

<details open class="snippet-subtrack">
<summary></summary>
{% include snippets/subtrack.md %}
</details>

<details open class="snippet-spinestart">
<summary></summary>
{% include snippets/spinestart.md %}
</details>

<details open class="snippet-nextcount">
<summary></summary>
{% include snippets/nextcount.md %}
</details>



