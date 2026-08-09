# Third-Party Assets

This file records the provenance of artwork shipped with, or deliberately kept
out of, this repository.

---

## characters/bufo — excluded

**Status: excluded — explicitly third-party per upstream LICENSE**

The upstream repository (`anthropics/claude-desktop-buddy`) LICENSE explicitly
places this artwork outside its MIT grant:

> "The GIF assets in characters/bufo/ are from the community 'bufo' emoji set
> (https://bufo.zone). They are third-party artwork redistributed for
> convenience and are not covered by the MIT license above; all rights remain
> with their original creators."

`characters/bufo/` is therefore **not** redistributed here.

- Source reference: https://github.com/anthropics/claude-desktop-buddy/blob/main/LICENSE
- Action required before any inclusion: confirm redistribution terms with the
  original creators of the bufo emoji set.

**Note on the upstream MIT grant:** it covers the code only. Do not assume that
cloning an MIT-licensed project grants you its artwork — this repository's own
upstream is a worked example of a project where it does not.

---

## data/chars/claude — original work, MIT

**Status: original artwork by this repository's author. Covered by the project
MIT license.**

The pixel-art robot sprites under `data/chars/claude/` were drawn for this
project. They are not derived from `characters/bufo/`, not taken from the
upstream repository (which contains no `characters/claude/` directory), and not
Anthropic brand artwork.

They may be redistributed under the same MIT terms as the code.

### Correction

An earlier revision of this file listed these sprites as "excluded — provenance
and license scope unconfirmed," on the assumption that they had come from
upstream and might fall outside its MIT grant. That assumption was wrong: the
upstream repository has no such directory, and the sprites are original work.
The exclusion was a precaution taken under a mistaken premise, not a finding.

A related note about these files being retrievable from git history was written
under the same mistaken premise. Their presence in history is not a licensing
problem, because there is no third-party claim on them.

---

## Adding your own character assets

Place GIFs at `data/chars/<species>/<state>.gif` and flash them with:

```bash
pio run --target uploadfs
```

Expected states: `idle.gif`, `busy.gif`, `attention.gif`, `celebrate.gif`,
`sleep.gif`, `dizzy.gif`, `heart.gif`

If you bring artwork from elsewhere, record its licence here before committing
it. "It was in an MIT repository" is not by itself a licence to redistribute
artwork.
