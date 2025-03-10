Examples

```bash
$ curl 'http://localhost:1234/roq/top_of_book?source=deribit&exchange=deribit&symbol=BTC-PERPETUAL'

{"bid_price":82239,"ask_price":82255.5}
```

```bash
$ curl 'http://localhost:1234/roq/reference_data?source=deribit&exchange=deribit&symbol=BTC-PERPETUAL'
{"description":"","tick_size":0.5}
```

```bash
$ curl 'http://localhost:1234/roq/position?source=deribit&exchange=deribit&symbol=BTC-PERPETUAL&account=A1'
{"long_quantity":5,"short_quantity":0}
```
