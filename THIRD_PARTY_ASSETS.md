# Third-Party Assets

Character and artwork assets are not included in this repository by default
due to licensing uncertainty. Each entry below records the status and reason.

---

## characters/bufo

**Status: excluded — explicitly third-party per upstream LICENSE**

The upstream repository (`anthropics/claude-desktop-buddy`) LICENSE explicitly states:

> "Third-party artwork redistributed for convenience. All rights remain with their original creators."

`characters/bufo/` is called out by name as outside the MIT grant.

- Source reference: https://github.com/anthropics/claude-desktop-buddy/blob/main/LICENSE
- Action required: Confirm redistribution conditions with original creators before including.

---

## characters/claude

**Status: excluded — provenance and license scope unconfirmed**

The upstream LICENSE grants MIT for the repository's code, but does not explicitly
confirm that `characters/claude/` GIF assets fall within that MIT grant.
The LICENSE only names `characters/bufo/` as a third-party exception, leaving
the status of other artwork ambiguous.

Until provenance and license scope are confirmed, these assets are excluded
from distribution as a precaution.

- Source reference: https://github.com/anthropics/claude-desktop-buddy/blob/main/LICENSE
- Action required: Confirm with Anthropic whether these assets are MIT-licensed
  for redistribution in derivative works.

---

## How to add character assets locally

Place GIFs at `data/chars/<species>/<state>.gif` and flash with:

```bash
pio run --target uploadfs
```

Expected states: `idle.gif`, `busy.gif`, `attention.gif`, `celebrate.gif`,
`sleep.gif`, `dizzy.gif`, `heart.gif`
