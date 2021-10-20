import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PppoeComponent } from './pppoe.component';

describe('PppoeComponent', () => {
  let component: PppoeComponent;
  let fixture: ComponentFixture<PppoeComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PppoeComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PppoeComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
