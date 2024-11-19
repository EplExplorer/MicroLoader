meta:
  id: eform

seq:
  - id: form_infos
    type: form_infos

types:
  form_infos:
    seq:
      - id: block
        type: block_id_and_address
      - id: infos
        type: form_info
        repeat: expr
        repeat-expr: block.length / 8
    doc: 窗口信息列表

  form_info:
    seq:
      - id: unk_before_class
        type: u4le
      - id: class
        type: u4le
      - id: name
        type: string_with_length_prefix
      - id: comment
        type: string_with_length_prefix
      - id: form_elements
        type: form_elements
    doc: 窗口信息

  form_elements:
    seq:
      - id: block
        type: block_id_and_offest
      - id: form_element
        type: form_element
        repeat: expr
        repeat-expr: block.count
    doc: 组件信息列表
  
  form_element:
    seq:
      - id: length
        type: u4le
      - id: data_type
        type: u4le
        doc: 组件类型
      - id: data
        type: u1
        repeat: expr
        repeat-expr: length - 4
        doc: 组件属性数据
    doc: 组件信息
  
  string_with_length_prefix:
    seq:
      - id: length
        type: u4le
      - id: data
        type: u1
        repeat: expr
        repeat-expr: length
    doc: 长度+字符串

  block_id_and_offest:
    seq:
      - id: count
        type: u4le
      - id: size
        type: u4le
      - id: id_list
        type: u4le
        repeat: expr
        repeat-expr: count
      - id: offset_list
        type: u4le
        repeat: expr
        repeat-expr: count
    doc: 头部带有id和偏移量的块

  block_id_and_address:
    seq:
      - id: length
        type: u4le
      - id: id_list
        type: u4le
        repeat: expr
        repeat-expr: length / 8
      - id: address_list
        type: u4le
        repeat: expr
        repeat-expr: length / 8
    doc: 头部带有id和地址的块
    