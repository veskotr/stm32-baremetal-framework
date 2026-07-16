# Architecture Decision Records

Decision records capture durable choices that future changes must understand. They complement the architecture and roadmap: architecture describes the current system, while a decision record explains why a constraint exists and what would be required to change it.

## Status values

- `Proposed`: under discussion and not binding.
- `Accepted`: current direction.
- `Superseded`: replaced by a later record.
- `Rejected`: considered but not adopted.

## Index

| ID | Decision | Status |
| --- | --- | --- |
| [0001](0001-record-framework-baseline.md) | Record the existing framework baseline | Accepted |
| [0002](0002-add-watchdog-helper-and-board-role.md) | Add watchdog helper and board role | Accepted |
| [0003](0003-separate-framework-components-and-products.md) | Separate framework, components, and products | Accepted |

## Process

Create a record when a change establishes a durable constraint, affects several modules or consumers, selects between meaningful alternatives, or reverses existing direction. Small implementation details do not need a record.

1. Copy `0000-template.md` to the next four-digit number and a short kebab-case title.
2. Write the context, decision, consequences, and credible alternatives.
3. Add it to this index.
4. If it supersedes another record, update both records.
5. Update architecture or workflow docs when the accepted decision changes the current system description.

Decision records are append-only history after acceptance. Correct typos and links directly; supersede substantive decisions with a new record.
