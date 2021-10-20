import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { DdnsComponent } from './ddns.component';

describe('DdnsComponent', () => {
  let component: DdnsComponent;
  let fixture: ComponentFixture<DdnsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DdnsComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DdnsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
