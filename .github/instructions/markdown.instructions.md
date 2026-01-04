---
name: Markdown Development Instructions
description: Guidelines and best practices for writing Markdown files.
applyTo: "**/*.md"
---

# Markdown Development Instructions

## General Guidelines

- **Use semantic structure:** Start with a single H1 heading (`#`) per document, followed by H2 (`##`) and H3 (`###`)
  as needed. Avoid skipping heading levels.
- **Keep lines short:** Limit lines to 120 characters for readability and easier diffs.
- **Consistent indentation:** Use two spaces for nested lists and code blocks.
- **Write in plain English:** Use clear, concise language. Prefer active voice and direct instructions.
- **Use Australian English:** Follow Australian spelling and grammar conventions.

## Formatting

- **Emphasise appropriately:** Use `*italic*` for emphasis and `**bold**` for strong importance.
- **Code blocks:** Use fenced code blocks (```) with language identifiers for syntax highlighting. Code blocks should
  wrap at 80 characters.
- **Inline code:** Use backticks (`) for inline code snippets.
- **Lists:** Use `-` for unordered lists and `1.` for ordered lists. Indent nested lists by two spaces.
- **Links:** Use descriptive link text, not raw URLs. Example: `[GitHub](https://github.com/)`. Use relative links for
  internal references within the repository.
- **Images:** Always provide alt text. Example: `![Project logo](images/logo.png "Logo")`
- **Tables:** Use pipes (`|`) and dashes (`-`) for tables. Align columns for readability.

## Content
- **Avoid Redundancy:** Be concise and avoid repeating information elsewhere in the document and in other documents.
  Use links to reference related content.
- **Use Lists for Clarity:** Prefer lists over long paragraphs when enumerating items or steps.
- **Consistent Terminology:** Use consistent terms and phrases throughout the document to avoid confusion.
