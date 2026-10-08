# Model format fixtures

`model-v17.ram` and `model-v17.rbm` are original MODEL 1.3.17 files generated
before WF02 changed the model format. They were produced from
`stage9_truss4.ram` by the old `range-workflow-native prepare` command with
passive control, `dt=0.001`, and two steps.

Provenance at generation time:

- Range FEA root revision: `9ecad58`
- `range-model-lib` revision: `eb3cb99`
- ASCII SHA-256: `14d4e0aa5305021166adfc9c577dbe10eb732981d2cd3b1bee164dc5d8e4888b`
- Binary SHA-256: `3e8542451e42c6c551ea937a6f57ced641571252dfa2919fbc10c2dd2c0c203b`

Keep these fixture bytes unchanged. They verify real 1.3.17 compatibility; tests
that synthesize 1.3.16 files derive them from these originals and only remove
the node and element physical-identifier table introduced in 1.3.17.
