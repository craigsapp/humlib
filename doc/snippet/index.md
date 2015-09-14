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

h1.h1-snippet {
	color: #871955;

}

</style>

<h1 class="h1-snippet"> Reading/Writing </h1>

<details class="snippet-read">
<summary></summary>
{% include snippets/read.md %}
</details>

<h1 class="h1-snippet"> Data access </h1>

<details class="snippet-token">
<summary></summary>
{% include snippets/token.md %}
</details>

<details class="snippet-tokstring">
<summary></summary>
{% include snippets/tokstring.md %}
</details>

<details class="snippet-spinestart">
<summary></summary>
{% include snippets/spinestart.md %}
</details>

<h1 class="h1-snippet"> Structural information </h1>

<details class="snippet-linecount">
<summary></summary>
{% include snippets/linecount.md %}
</details>

<details class="snippet-fieldcount">
<summary></summary>
{% include snippets/fieldcount.md %}
</details>

<details class="snippet-track">
<summary></summary>
{% include snippets/track.md %}
</details>

<details class="snippet-subtrack">
<summary></summary>
{% include snippets/subtrack.md %}
</details>

<details class="snippet-nextcount">
<summary></summary>
{% include snippets/nextcount.md %}
</details>

<h1 class="h1-snippet"> Timing information </h1>

<details class="snippet-tokdur">
<summary></summary>
{% include snippets/tokdur.md %}
</details>

<details class="snippet-linedur">
<summary></summary>
{% include snippets/linedur.md %}
</details>

<details class="snippet-filedur">
<summary></summary>
{% include snippets/filedur.md %}
</details>

<details class="snippet-tpq">
<summary></summary>
{% include snippets/tpq.md %}
</details>

<details class="snippet-tpqdur">
<summary></summary>
{% include snippets/tpqdur.md %}
</details>


