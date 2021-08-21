#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_variables)]
use crate::field::raw_to_value;
use crate::field::FieldValue;
use crate::formula::FormulaData;
use crate::sheet::Sheet;
use crate::ui_table::TableView;
use csv::StringRecord;
use cursive::theme::BaseColor;
use cursive::theme::Color;
use cursive::theme::Color::RgbLowRes;
use cursive::theme::ColorStyle;
use cursive::views::TextView;
use cursive::Vec2;
use rhai::{ASTNode, Engine, Expr, AST};
use smartstring;
use smartstring::Compact;
use std::error::Error;
use std::io;

mod field;
mod formula;
mod sheet;
mod ui_table;

fn main() -> Result<(), Box<dyn Error>> {
    let engine = Engine::new();
    //println!("Input a formula");
    // let mut input = String::new();
    //io::stdin()
    //    .read_line(&mut input)
    //    .expect("Failed to read input.");
    //let ast = engine.compile_expression(&input)?;
    //let all_vars = collect_all_variables(&ast);
    //println!("Your input {}", input);
    //println!("all vars {:#?}", all_vars);

    let mut my_sheet = Sheet::create();
    load_file("test.csv", &mut my_sheet);

    let mut sheet2 = Sheet2::create();
    load_file_2("test.csv", &mut sheet2);

    let mut siv = cursive::default();

    siv.add_global_callback('q', |s| s.quit());

    //siv.add_layer(TextView::new("Hello cursive! Press <q> to quit."));

    siv.add_fullscreen_layer(TableView2 { sheet: sheet2 });

    siv.run();

    Ok(())
}

fn load_file<'a>(name: &str, sheet: &mut sheet::Sheet<'a>) {
    let mut rdr = csv::ReaderBuilder::new()
        .has_headers(false)
        .from_path(name)
        .expect("Error open file");

    rdr.records().enumerate().for_each(|(y, record)| {
        let record = record.unwrap().clone();
        record.iter().enumerate().for_each(|(x, raw)| {
            //sheet.data[x][y].value = raw_to_value(raw);
        })
    })
}

/// =========================================
/// =========================================
///

pub struct FormulaData2 {
    text: String,
}

pub enum FormValue {
    Empty(),
    Number(i32),
    Text(String),
    Formula(FormulaData2),
}

pub struct Sheet2 {
    pub data: Vec<Vec<FormValue>>,
}

const MAX_SIZE: i32 = 24;

impl Sheet2 {
    pub fn create() -> Sheet2 {
        let mut data: Vec<Vec<FormValue>> = Vec::new();
        for _ in 0..MAX_SIZE {
            let mut vect: Vec<FormValue> = Vec::new();
            for _ in 0..MAX_SIZE {
                let value = FormValue::Empty();
                vect.push(value);
            }
            data.push(vect);
        }
        Sheet2 { data }
    }
}

fn load_file_2(name: &str, sheet: &mut Sheet2) {
    let mut rdr = csv::ReaderBuilder::new()
        .has_headers(false)
        .from_path(name)
        .expect("Error open file");
    rdr.records().enumerate().for_each(|(y, record)| {
        let record = record.unwrap().clone();
        record.iter().enumerate().for_each(|(x, raw)| {
            let raw_value = String::from(raw);
            sheet.data[x][y] = raw_to_form_value(raw_value);
        })
    })
}

fn raw_to_form_value(raw_value: String) -> FormValue {
    if raw_value.is_empty() {
        FormValue::Empty()
    } else if raw_value.starts_with("=") {
        FormValue::Formula(FormulaData2 { text: raw_value })
    } else {
        match raw_value.parse::<i32>() {
            Ok(n) => FormValue::Number(n),
            Err(_) => FormValue::Text(raw_value),
        }
    }
}

fn init_ui() {}

fn collect_all_variables(ast: &AST) -> Vec<smartstring::SmartString<Compact>> {
    let mut result = Vec::new();
    ast.walk(&mut |path| -> bool {
        match path.last().expect("Always current node") {
            ASTNode::Expr(Expr::Variable(_, _, name)) => {
                let (_, _, id) = (**name).clone();
                result.push(id);
                true
            }
            _ => true,
        }
    });
    result
}

#[cfg(test)]
mod test {
    //  use super::*;
    #[test]
    fn noop() {
        assert!(true);
    }
}

pub struct TableView2 {
    pub sheet: Sheet2,
}

impl cursive::view::View for TableView2 {
    fn draw(&self, printer: &cursive::Printer<'_, '_>) {
        for (x, column) in self.sheet.data.iter().enumerate() {
            let name = "ABCDEFGHIJKLMNOPQRSTUVW".chars().nth(x).get_or_insert('_').to_string();
            let x = x * 30;
            printer.with_color(
                ColorStyle::from(RgbLowRes(5, 1, 1)), |printer| {
                            printer.print((x, 0), &name )
                        });
            for (y, cell) in column.iter().enumerate() {
                let y = y + 1;
                match cell {
                    FormValue::Text(str) => {
                        printer.print((x, y), str);
                    }
                    FormValue::Number(n) => printer
                        .with_color(ColorStyle::secondary(), |printer| {
                            printer.print((x, y), &n.to_string())
                        }),
                    FormValue::Formula(FormulaData2 { text }) => printer
                        .with_color(ColorStyle::from(RgbLowRes(5, 1, 1)), |printer| {
                            printer.print((x, y), &text)
                        }),
                    _ => {
                        printer.print((x, y), "");
                    }
                };
            }
        }
    }

    fn required_size(&mut self, _: Vec2) -> Vec2 {
        Vec2::new(200, 40)
    }
}
